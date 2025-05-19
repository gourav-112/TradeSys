#include "models/slippage.h"

double SlippageModel::estimate(double usdQty) {
    return usdQty * 0.0005; // Placeholder: 5bps
}
