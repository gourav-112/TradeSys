#include "models/impact.h"

double MarketImpactModel::compute(double qty, double duration) {
    return 0.01 * qty / duration; // Simplified model
}
