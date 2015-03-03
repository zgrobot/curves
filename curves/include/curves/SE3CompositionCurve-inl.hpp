/*
 * @file CompositionCurve-inl.hpp
 * @date Feb 06, 2015
 * @author Renaud Dubé, Abel Gawel, Mike Bosse
 */

// COMPOSITION_STRATEGY is used to select the strategy for composing the
// correction curve with the base curve at the evaluation time t:
// (1) corr(t) * base(t) is implemented
// (2) corr is evaluated at the coefficient times (t1, t2) where the interpolation
//     on base is computed resulting in
//     interpolation(corr(t1) * base(t1), corr(t2) * base(t2), alpha)

#define COMPOSITION_STRATEGY 1

#include "SE3CompositionCurve.hpp"
#include "../test/test_Helpers.hpp"

namespace curves{

template <class C1, class C2>
SE3CompositionCurve<C1, C2>::SE3CompositionCurve() {

}

template <class C1, class C2>
SE3CompositionCurve<C1, C2>::~SE3CompositionCurve() {

}
template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::print(const std::string& str) const {
  std::cout << "=========================================" << std::endl;
  std::cout << "===== CompositionCurve SE3 CURVE ========" << std::endl;
  std::cout << str << std::endl;
  baseCurve_.print("Base curve");
  correctionCurve_.print("Correction curve");
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::saveCurves(const std::string& filename) const {
  std::vector<Time> baseCurveTimes;
  baseCurve_.manager_.getTimes(&baseCurveTimes);

  Eigen::VectorXd v(7);

  std::vector<Eigen::VectorXd> baseCurveValues;
  for (size_t i = 0; i < baseCurveTimes.size(); ++i) {
    ValueType val = baseCurve_.evaluate(baseCurveTimes[i]);
    v << val.getPosition().x(), val.getPosition().y(), val.getPosition().z(),
        val.getRotation().w(), val.getRotation().x(), val.getRotation().y(), val.getRotation().z();
    baseCurveValues.push_back(v);
  }

  std::vector<Time> correctionCurveTimes;
  correctionCurve_.manager_.getTimes(&correctionCurveTimes);
  std::vector<Eigen::VectorXd> correctionCurveValues;
  for (size_t i = 0; i < correctionCurveTimes.size(); ++i) {
    ValueType val = correctionCurve_.evaluate(correctionCurveTimes[i]);
    v << val.getPosition().x(), val.getPosition().y(), val.getPosition().z(),
        val.getRotation().w(), val.getRotation().x(), val.getRotation().y(), val.getRotation().z();
    correctionCurveValues.push_back(v);
  }

  std::vector<Eigen::VectorXd> combinedCurveValues;
  for (size_t i = 0; i < baseCurveTimes.size(); ++i) {
    ValueType val = this->evaluate(baseCurveTimes[i]);
    v << val.getPosition().x(), val.getPosition().y(), val.getPosition().z(),
        val.getRotation().w(), val.getRotation().x(), val.getRotation().y(), val.getRotation().z();
    combinedCurveValues.push_back(v);
  }

  CurvesTestHelpers::writeTimeVectorCSV(filename + "_base.csv", baseCurveTimes, baseCurveValues);
  CurvesTestHelpers::writeTimeVectorCSV(filename + "_correction.csv", correctionCurveTimes, correctionCurveValues);
  CurvesTestHelpers::writeTimeVectorCSV(filename + "_composed.csv", baseCurveTimes, combinedCurveValues);

}

template <class C1, class C2>
Time SE3CompositionCurve<C1, C2>::getMinTime() const{
  return baseCurve_.getMinTime();
}

template <class C1, class C2>
Time SE3CompositionCurve<C1, C2>::getMaxTime() const{
  return baseCurve_.getMaxTime();
}

template <class C1, class C2>
bool SE3CompositionCurve<C1, C2>::isEmpty() const{
  return baseCurve_.isEmpty();
}

template <class C1, class C2>
int SE3CompositionCurve<C1, C2>::size() const{
//  return baseCurve_.size();
  return correctionCurve_.size();
}

template <class C1, class C2>
int SE3CompositionCurve<C1, C2>::correctionSize() const{
  return correctionCurve_.size();
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::setMinSamplingPeriod(const Time minSamplingPeriod) {
  baseCurve_.setMinSamplingPeriod(0);
  correctionCurve_.setMinSamplingPeriod(minSamplingPeriod);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::extend(const std::vector<Time>& times,
                                         const std::vector<typename SE3CompositionCurve<C1, C2>::ValueType>& values,
                                         std::vector<Key>* outKeys) {
  //todo: Treat case when times.size() != 1
  CHECK_EQ(times.size(), 1) << "Extend was called with more than one time.";
  CHECK_EQ(values.size(), 1) << "Extend was called with more than one value.";

  // Find the new limit times of the curve
  Time newMaxTime, newMinTime;

  if (baseCurve_.isEmpty()) {
    newMaxTime = 0;
    newMinTime = 0;
  } else {
    newMaxTime = baseCurve_.getMaxTime();
    newMinTime  = baseCurve_.getMinTime();
  }

  for (std::vector<Time>::const_iterator it = times.begin(); it != times.end(); ++it) {
    if (newMaxTime < *it) {
      newMaxTime = *it;
    }
    if (newMinTime > *it) {
      newMinTime = *it;
    }
  }

  // Extend the correction curves to these times
  std::vector<Time> correctionTimes;
  std::vector<ValueType> correctionValues;

  if (correctionCurve_.isEmpty()) {
    correctionTimes.push_back(newMinTime);
    correctionValues.push_back(ValueType(ValueType::Position(0,0,0), ValueType::Rotation(1,0,0,0)));
    correctionCurve_.extend(correctionTimes, correctionValues);
  }

  if (correctionCurve_.getMaxTime() < newMaxTime) {
    correctionTimes.push_back(newMaxTime);
    correctionValues.push_back(correctionCurve_.evaluate(correctionCurve_.getMaxTime()));
    correctionCurve_.extend(correctionTimes, correctionValues);
  }

  if (correctionCurve_.getMinTime() > newMinTime) {
    correctionTimes.push_back(newMinTime);
    correctionValues.push_back(correctionCurve_.evaluate(correctionCurve_.getMinTime()));
    correctionCurve_.extend(correctionTimes, correctionValues);
  }

  //Compute the base curve updates accounting for the corrections
  std::vector<ValueType> newValues;
  std::vector<ValueType>::const_iterator itValues = values.begin();
  for (std::vector<Time>::const_iterator it = times.begin(); it != times.end(); ++it) {
    newValues.push_back(correctionCurve_.evaluate(*it).inverted() * (*itValues));
    ++itValues;
  }
  baseCurve_.extend(times, newValues, outKeys);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::foldInCorrections() {
  std::cout << "foldincorrections" << std::endl;
  std::vector<Time> times;
  std::vector<ValueType> newValues;
  baseCurve_.manager_.getTimes(&times);
  for (std::vector<Time>::const_iterator it = times.begin(); it != times.end(); ++it) {
    newValues.push_back(this->evaluate(*it));
  }
  baseCurve_.clear();
  baseCurve_.extend(times,newValues);
  times.clear();
  newValues.clear();
  correctionCurve_.manager_.getTimes(&times);
  for (std::vector<Time>::const_iterator it = times.begin(); it != times.end(); ++it) {
    newValues.push_back(ValueType(ValueType::Position(0,0,0), ValueType::Rotation(1,0,0,0)));
  }
  correctionCurve_.clear();
  correctionCurve_.extend(times,newValues);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::fitCurve(const std::vector<Time>& times,
                                           const std::vector<typename SE3CompositionCurve<C1, C2>::ValueType>& values,
                                           std::vector<Key>* outKeys){
  extend(times, values, outKeys);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::setCorrectionTimes(const std::vector<Time>& times) {
  // Evaluate the correction curve at these times
  std::vector<ValueType> values;
  for (size_t i = 0; i < times.size(); ++i) {
    values.push_back(correctionCurve_.evaluate(times[i]));
  }

  // Redefine the correction curve
  correctionCurve_.clear();
  correctionCurve_.extend(times, values);

  CHECK_EQ(correctionCurve_.getMinTime(), baseCurve_.getMinTime()) << "Min time of correction curve and base curve are different";
  CHECK_EQ(correctionCurve_.getMaxTime(), baseCurve_.getMaxTime()) << "Min time of correction curve and base curve are different";
}

template <class C1, class C2>
typename SE3CompositionCurve<C1, C2>::ValueType SE3CompositionCurve<C1, C2>::evaluate(Time time) const{

#if COMPOSITION_STRATEGY == 1
  // (1) corr(t) * base(t) is implemented
  return correctionCurve_.evaluate(time) * baseCurve_.evaluate(time);
#elif COMPOSITION_STRATEGY == 2
  // (2) corr is evaluated at the coefficient times (t1, t2) where the interpolation
  //     on base is computed resulting in
  //     interpolation(corr(t1) * base(t1), corr(t2) * base(t2), alpha)
  if (time == 0) {
    return correctionCurve_.evaluate(time) * baseCurve_.evaluate(time);
  }

  typename C2::CoefficientIter it1, it2;
  baseCurve_.manager_.getCoefficientsAt(time, &it1, &it2);
  //todo Is it true that the value of a curve at a time where a coefficient
  // is defined is always only dependent on this coefficient?
  if(it1->first == time || it2->first == time) {
    return correctionCurve_.evaluate(time) * baseCurve_.evaluate(time);
  } else {
    Time tA, tB;
    ValueType A, B, dA, dB, T_W_A, T_W_B;
    //if the iterators returned are in order ( last element bug)
    if (it1->first < it2->first) {
      tA = it1->first;
      tB = it2->first;
    } else {
      tA = it2->first;
      tB = it1->first;
    }

    double alpha = double(time - tA)/double(tB - tA);

    A = baseCurve_.evaluate(tA);
    B = baseCurve_.evaluate(tB);
    dA = correctionCurve_.evaluate(tA);
    dB = correctionCurve_.evaluate(tB);

    T_W_A = dA * A;
    T_W_B = dB * B;

    //Implementation of T_W_I = T_W_A*exp(alpha*log(inv(T_W_A)*T_W_B))
    using namespace kindr::minimal;
    ValueType T_A_B = invertAndComposeImplementation(T_W_A, T_W_B, boost::none, boost::none);
    gtsam::Vector6 log_T_A_B = transformationLogImplementation(T_A_B, boost::none);
    gtsam::Vector6 log_T_A_I = vectorScalingImplementation<int(6)>(log_T_A_B, alpha, boost::none, boost::none);
    ValueType T_A_I = transformationExpImplementation(log_T_A_I, boost::none);
    return composeImplementation(T_W_A, T_A_I, boost::none, boost::none);
  }
#endif
}

template <class C1, class C2>
gtsam::Expression<typename SE3CompositionCurve<C1, C2>::ValueType>
SE3CompositionCurve<C1, C2>::getValueExpression(const Time& time) const{

#if COMPOSITION_STRATEGY == 1
  // (1) corr(t) * base(t) is implemented
  return kindr::minimal::compose(correctionCurve_.getValueExpression(time),
                                 gtsam::Expression<ValueType>(baseCurve_.evaluate(time)));
#elif COMPOSITION_STRATEGY == 2
  // (2) corr is evaluated at the coefficient times (t1, t2) where the interpolation
  //     on base is computed resulting in
  //     interpolation(corr(t1) * base(t1), corr(t2) * base(t2), alpha)
  typename C2::CoefficientIter it1, it2;
  baseCurve_.manager_.getCoefficientsAt(time, &it1, &it2);
  if(it1->first == time || it2->first == time) {
    return kindr::minimal::compose(correctionCurve_.getValueExpression(time),
                                   gtsam::Expression<ValueType>(baseCurve_.evaluate(time)));
  } else {
    //if the iterators returned are in order (last element bug)
    Time tA, tB;
    if (it1->first < it2->first) {
      tA = it1->first;
      tB = it2->first;
    } else {
      tA = it2->first;
      tB = it1->first;
    }

    gtsam::Expression<ValueType> E_dA = correctionCurve_.getValueExpression(tA);
    gtsam::Expression<ValueType> E_dB = correctionCurve_.getValueExpression(tB);

    gtsam::Expression<ValueType> E_A(baseCurve_.evaluate(tA));
    gtsam::Expression<ValueType> E_B(baseCurve_.evaluate(tB));

    gtsam::Expression<ValueType> E_dAA = kindr::minimal::compose(E_dA, E_A);
    gtsam::Expression<ValueType> E_dBB = kindr::minimal::compose(E_dB, E_B);

    double alpha = double(time - tA)/double(tB - tA);

    return kindr::minimal::slerp(E_dAA, E_dBB, alpha);
  }
#endif
}

template <class C1, class C2>
typename SE3CompositionCurve<C1, C2>::DerivativeType SE3CompositionCurve<C1, C2>::evaluateDerivative(Time time,
                                                                                                     unsigned derivativeOrder) const{
  //todo
}

template <class C1, class C2>
gtsam::Expression<typename SE3CompositionCurve<C1, C2>::DerivativeType>
SE3CompositionCurve<C1, C2>::getDerivativeExpression(const Time& time, unsigned derivativeOrder) const{
  //todo
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::setTimeRange(Time minTime, Time maxTime){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateAngularVelocityA(Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateAngularVelocityB(Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateLinearVelocityA(Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateLinearVelocityB(Time time){
  //todo
}

template <class C1, class C2>
Vector6d SE3CompositionCurve<C1, C2>::evaluateTwistA(Time time){
  //todo
}

template <class C1, class C2>
Vector6d SE3CompositionCurve<C1, C2>::evaluateTwistB(Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateAngularDerivativeA(unsigned derivativeOrder, Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateAngularDerivativeB(unsigned derivativeOrder, Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateLinearDerivativeA(unsigned derivativeOrder, Time time){
  //todo
}

template <class C1, class C2>
Eigen::Vector3d SE3CompositionCurve<C1, C2>::evaluateLinearDerivativeB(unsigned derivativeOrder, Time time){
  //todo
}

template <class C1, class C2>
Vector6d SE3CompositionCurve<C1, C2>::evaluateDerivativeA(unsigned derivativeOrder, Time time){
  //todo
}

template <class C1, class C2>
Vector6d SE3CompositionCurve<C1, C2>::evaluateDerivativeB(unsigned derivativeOrder, Time time){
  //todo
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::initializeGTSAMValues(gtsam::FastVector<gtsam::Key> keys, gtsam::Values* values) const{
  correctionCurve_.initializeGTSAMValues(keys, values);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::initializeGTSAMValues(gtsam::Values* values) const{
  correctionCurve_.initializeGTSAMValues(values);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::updateFromGTSAMValues(const gtsam::Values& values){
  correctionCurve_.updateFromGTSAMValues(values);
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::clear(){
  baseCurve_.clear();
  correctionCurve_.clear();
}

template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::removeCorrectionCoefficientAtTime(Time time) {
  CHECK(correctionCurve_.manager_.hasCoefficientAtTime(time));
  correctionCurve_.manager_.removeCoefficientAtTime(time);
}
template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::setCorrectionCoefficientAtTime(Time time, ValueType value) {
  CHECK(correctionCurve_.manager_.hasCoefficientAtTime(time));
  correctionCurve_.manager_.insertCoefficient(time, value);
}
template <class C1, class C2>
void SE3CompositionCurve<C1, C2>::addPriorFactors(gtsam::NonlinearFactorGraph* graph,
                                                  Time priorTime) const {
  correctionCurve_.addPriorFactors(graph, priorTime);
}

} // namespace curves