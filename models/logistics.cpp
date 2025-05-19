#include "models/logistic.h"
#include <cmath>

double LogisticRegression::predictProbability(double feature) {
    return 1.0 / (1.0 + std::exp(-feature)); // Sigmoid
}
