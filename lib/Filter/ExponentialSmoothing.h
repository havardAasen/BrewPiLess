#ifndef BREWPILESS_EXPONENTIALSMOOTHING_H
#define BREWPILESS_EXPONENTIALSMOOTHING_H

/**
 * @addtogroup filters
 * @{
 *
 * @class ExponentialSmoothing
 * @brief Simple exponential smoothing (single‑pole IIR) filter.
 *
 * The filter maintains an internal state \f$S_t\f$ that is updated with each
 * new sample \f$x_t\f$ according to
 *
 * \f[
 * S_t = S_{t-1} + \alpha (x_t - S_{t-1})
 * \f]
 *
 * where
 *
* \f[
 * 0 < \alpha \le 1
 * \f]
 *
 * Smaller \f$\alpha\f$ gives more smoothing (longer memory); \f$\alpha = 1.0\f$
 * passes the input through unchanged.
 */
class ExponentialSmoothing
{
public:
    explicit ExponentialSmoothing(const float alpha = 0.1f)
        : output_(0.0f), alpha_(alpha) {}

    void setInitial(const float value) noexcept { output_ = value; }

    void setAlpha(const float alpha) noexcept { alpha_ = alpha; }
    [[nodiscard]] float alpha() const noexcept { return alpha_; }
    [[nodiscard]] float addSample(const float x) noexcept
    {
        output_ += alpha_ * (x - output_);
        return output_;
    }

    /** Retrieve the current filtered value without updating. */
    [[nodiscard]] float output() const noexcept { return output_; }

private:
    float output_;   ///< Current filtered output \f$S_{n-1}\f$
    float alpha_;    ///< Smoothing factor \f$\alpha\f$
};

#endif
