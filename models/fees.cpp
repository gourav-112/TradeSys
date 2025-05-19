#include "models/fees.h"

double FeeModel::calculate(double notional) {
    return notional * 0.001; // 0.1% fee
}
