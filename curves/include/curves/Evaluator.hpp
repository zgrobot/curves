#ifndef CURVES_TYPED_EVALUATOR_HPP
#define CURVES_TYPED_EVALUATOR_HPP

#include "EvaluatorBase.hpp"

namespace curves {

template<typename CurveConfig>
class Evaluator : public EvaluatorBase
{
 public:
  /// The value type of the curve.
  typedef typename CurveConfig::ValueType ValueType;

  /// The curve's derivative type.
  typedef typename CurveConfig::DerivativeType DerivativeType;

  /// A pointer type for this evaluator
  typedef boost::shared_ptr< Evaluator<CurveConfig> > Ptr;

  /// A pointer type for this evaluator
  typedef boost::shared_ptr< const Evaluator<CurveConfig> > ConstPtr;

  Evaluator() { }
  virtual ~Evaluator() { }

  /// Evaluate the ambient space of the curve (functional form).
  virtual ValueType evaluate(const std::vector<Coefficient>& coefficients) const = 0;
  
  /// Evaluate the curve derivatives (functional form).
  virtual Eigen::VectorXd evaluateDerivative(unsigned derivativeOrder, const std::vector<Coefficient>& coefficients) const = 0;

  /// Evaluate the ambient space of the curve (functional form).
  virtual ValueType evaluateAndJacobian(const std::vector<Coefficient>& coefficients, std::vector<Eigen::MatrixXd>& outJacobian) const = 0;
  
  /// Evaluate the curve derivatives (functional form).
  virtual Eigen::VectorXd evaluateDerivativeAndJacobian(unsigned derivativeOrder, const std::vector<Coefficient>& coefficients, std::vector<Eigen::MatrixXd>& outJacobian) const = 0;

  /// Get the maximum derivative order supported by this evaluator.
  size_t getMaximumDerivativeOrder() const;

};

} // namespace 

#endif /* CURVES_TYPED_EVALUATOR_HPP */
