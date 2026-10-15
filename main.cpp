#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <iomanip>
#include <numeric>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

struct StockData {
	vector<string> dates;
	vector<double> prices;
};

StockData readStockData(const string& filename) {
	StockData data;
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Error opening file: " << filename << endl;
		return data;
	}

	string line;
	getline(file, line); // Skip header
	while (getline(file, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		stringstream ss(line);
		string date, open, high, low, close, volume;
		getline(ss, date, ',');
		getline(ss, open, ',');
		getline(ss, high, ',');
		getline(ss, low, ',');
		getline(ss, close, ',');
		getline(ss, volume, ',');

		if (date.empty() || close.empty())
			continue;

		try {
			data.dates.push_back(date);
			data.prices.push_back(stod(close));
		}
		catch (const exception&) {
			cerr << "Bad line in " << filename << ": " << line << endl;
		}
	}
	return data;
}

StockData sliceStockData(const StockData& data, size_t s, size_t e) {
	StockData out;
	if (e > data.prices.size() || s > e) {
		cerr << "slice: invalid range [" << s << ", " << e
			<< ") for data of size " << data.prices.size() << '\n';
		return out;
	}

	for (size_t i = s; i < e; ++i) {
		out.dates.push_back(data.dates[i]);
		out.prices.push_back(data.prices[i]);
	}
	return out;
}

void dateAlign(StockData& a, StockData& b, StockData& c, int p) {
	map<string, double> ma, mb, mc;
	for (size_t i = 0; i < a.dates.size() / p; ++i) ma[a.dates[i]] = a.prices[i];
	for (size_t i = 0; i < b.dates.size() / p; ++i) mb[b.dates[i]] = b.prices[i];
	for (size_t i = 0; i < c.dates.size() / p; ++i) mc[c.dates[i]] = c.prices[i];

	StockData outA, outB, outC;
	for (const auto& entry : ma) {
		const string& date = entry.first;
		auto itB = mb.find(date);
		if (itB == mb.end()) continue;
		auto itC = mc.find(date);
		if (itC == mc.end()) continue;
		outA.dates.push_back(date);
		outA.prices.push_back(entry.second);
		outB.dates.push_back(date);
		outB.prices.push_back(itB->second);
		outC.dates.push_back(date);
		outC.prices.push_back(itC->second);
	}
	a = outA;
	b = outB;
	c = outC;
}

vector<double> computeReturns(const vector<double>& values) {
	vector<double> returns;
	if (values.size() < 2)
		return returns;

	for (size_t i = 1; i < values.size(); ++i) {
		returns.push_back((values[i] - values[i - 1]) / values[i - 1]);
	}
	return returns;
}

double computeMean(const vector<double>& values) {
	if (values.size() < 1)
		return 0.0;

	return accumulate(values.begin(), values.end(), 0.0) / values.size();
}

double computeVariance(const vector<double>& values) {
	if (values.size() < 2)
		return 0.0;

	double mean = computeMean(values);
	double sumsq = 0.0;
	for (size_t i = 0; i < values.size(); ++i) {
		sumsq += (values[i] - mean) * (values[i] - mean);
	}
	return sumsq / (values.size() - 1);
}

double computeSD(const vector<double>& values) {
	return sqrt(computeVariance(values));
}

double computeCovariance(const vector<double>& x, const vector<double>& y) {
	if (x.size() != y.size() || x.size() < 2)
		return 0.0;

	double meanX = computeMean(x);
	double meanY = computeMean(y);
	double sum = 0.0;

	for (size_t i = 0; i < x.size(); ++i) {
		sum += (x[i] - meanX) * (y[i] - meanY);
	}
	return sum / (x.size() - 1);
}

double computeCorr(const vector<double>& x, const vector<double>& y) {
	if (x.size() != y.size() || x.size() < 2)
		return 0.0;

	if (computeSD(x) == 0.0 || computeSD(y) == 0.0)
		return 0.0;

	return computeCovariance(x, y) / (computeSD(x) * computeSD(y));
}

vector<vector<double>> computeCovMatrix(const vector<vector<double>>& returns) {
	size_t n = returns.size();
	vector<vector<double>> covMatrix(n, vector<double>(n, 0.0));
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			covMatrix[i][j] = computeCovariance(returns[i], returns[j]);
		}
	}
	return covMatrix;
}

double computePortfolioVariance(const vector<double>& weights, const vector<vector<double>>& covMatrix) {
	size_t n = weights.size();
	double variance = 0.0;
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			variance += weights[i] * weights[j] * covMatrix[i][j];
		}
	}
	return variance;
}

double computePortfolioSD(const vector<double>& weights, const vector<vector<double>>& covMatrix) {
	return sqrt(computePortfolioVariance(weights, covMatrix));
}

vector<double> optimizeWeights(const vector<vector<double>>& covMatrix) {
	size_t n = covMatrix.size();
	size_t steps = 100;
	double minSD = 1e18;
	vector<double> bestWeights(n, 0.0);
	for (size_t i = 0; i < steps; ++i) {
		double w1 = double(i) / steps;
		for (size_t j = 0; j < steps; ++j) {
			double w2 = double(j) / steps;
			double w3 = 1.0 - w1 - w2;
			if (w3 < 0.05 || w2 < 0.05 || w1 < 0.05) // Setting boundaries to ensure each weight is at least 5%
				continue;
			if (w3 > 0.5 || w2 > 0.5 || w1 > 0.5) // Setting boundaries to ensure each weight is at most 50%
				continue;

			vector<double> temp = { w1, w2, w3 };
			if (computePortfolioSD(temp, covMatrix) < minSD) {
				minSD = computePortfolioSD(temp, covMatrix);
				bestWeights = temp;
			}
		}
	}

	return bestWeights;
}

double portfolioMean(const vector<double> weights, const vector<double> means) {
	double mean = 0.0;
	for (size_t i = 0; i < weights.size(); ++i) {
		mean += weights[i] * means[i];
	}
	return mean;
}

double computeSharpe(const vector<double> weights, const vector<double>& means, const vector<vector<double>> covM, double riskFree) {
	double excess = portfolioMean(weights, means) - riskFree;
	double sd = computePortfolioSD(weights, covM);

	// Guard
	if (sd == 0)
		return 0.0;

	return excess / sd;
}

vector<double> optimizeSharpe(const vector<double>& means, const vector<vector<double>> covM, double riskFree) {
	double maxSharpe = -1e18;
	size_t n = covM.size();
	vector<double> bestSharpe(n, 0.0);
	size_t steps = 100;

	for (size_t i = 0; i <= steps; ++i) {
		double w0 = double(i) / steps;
		for (size_t j = 0; j <= steps; ++j) {
			double w1 = double(j) / steps;
			double w2 = 1 - w0 - w1;

			if (w0 < 0.05 || w1 < 0.05 || w2 < 0.05)
				continue;
			if (w0 > 0.5 || w1 > 0.5 || w2 > 0.5)
				continue;

			vector<double> temp = { w0, w1, w2 };
			double sharpe = computeSharpe(temp, means, covM, riskFree);
			if (sharpe >= maxSharpe) {
				maxSharpe = sharpe;
				bestSharpe = temp;
			}
		}
	}

	return bestSharpe;
}

vector<double> gradientVariance(const vector<double>& weights, const vector<vector<double>>& covM) {
	size_t n = weights.size();
	vector<double> gradient(n, 0.0);
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			gradient[i] += 2 * weights[j] * covM[i][j];
		}
	}
	return gradient;
}

void project(vector<double>& weights) {
	double s = 0.0;
	for (double x : weights)
		s += x;

	for (double& x : weights)
		x /= s;
}

vector<double> gradientDescend(const vector<vector<double>> covM, int maxIter, double alpha) {
	size_t n = covM.size();
	vector<double> bestWeights(n, 1.0 / n);

	for (double& x : bestWeights) {
		if (x < 0.0)
			x = 0.0;
	}

	for (int iter = 0; iter < maxIter; ++iter) {
		vector<double> gradient = gradientVariance(bestWeights, covM);
		for (size_t i = 0; i < n; ++i) {
			bestWeights[i] -= alpha * gradient[i];
		}
		project(bestWeights);
	}
	return bestWeights;
}

void writeRow(const string& filename, const string& period, const string& strategy, const vector<double>& weights, double mean, double sd, double sharpe) {
	ofstream out(filename, ios::app);
	out << fixed << setprecision(6);
	out << period << "," << strategy << "," << weights[0] << "," << weights[1] << "," << weights[2] << "," << mean << "," << sd << "," << sharpe << "\n";
}

void output(ostream& out, const string& period, const string& filename, const vector<double>& a, const vector<double>& b, const vector<double>& c) {
	out << string(25, '=') << period << string(25, '=') << endl;

	out << "Mean return: " << '\n';
	out << "Nvidia: " << computeMean(a) << '\n';
	out << "AMD: " << computeMean(b) << '\n';
	out << "Intel: " << computeMean(c) << '\n';
	out << string(50, '-') << '\n';

	out << "Variance of returns: " << '\n';
	out << "Nvidia: " << computeVariance(a) << '\n';
	out << "AMD: " << computeVariance(b) << '\n';
	out << "Intel: " << computeVariance(c) << '\n';
	out << string(50, '-') << '\n';

	out << "Standard deviation of returns: " << '\n';
	out << "Nvidia: " << computeSD(a) << '\n';
	out << "AMD: " << computeSD(b) << '\n';
	out << "Intel: " << computeSD(c) << '\n';
	out << string(50, '-') << '\n';

	out << "Covariance of returns: " << '\n';
	out << "Nvidia-AMD: " << computeCovariance(a, b) << '\n';
	out << "Nvidia-Intel: " << computeCovariance(a, c) << '\n';
	out << "AMD-Intel: " << computeCovariance(b, c) << '\n';
	out << string(50, '-') << '\n';

	out << "Correlation of returns: " << '\n';
	out << "Nvidia-AMD: " << computeCorr(a, b) << '\n';
	out << "Nvidia-Intel: " << computeCorr(a, c) << '\n';
	out << "AMD-Intel: " << computeCorr(b, c) << '\n';
	out << string(50, '-') << '\n';

	vector<vector<double>> covMatrix = computeCovMatrix({ a, b, c });
	out << "Covariance matrix: " << '\n';
	for (const auto& row : covMatrix) {
		for (double val : row) {
			out << val << " ";
		}
		out << endl;
	}
	out << string(50, '-') << '\n';

	vector<double> weights = { 0.4, 0.4, 0.2 }; // Example weights for Nvidia, AMD, Intel
	double portfolioVariance = computePortfolioVariance(weights, covMatrix);
	double portfolioSD = computePortfolioSD(weights, covMatrix);
	out << "Portfolio Weights: Nvidia: " << weights[0] << ", AMD: " << weights[1] << ", Intel: " << weights[2] << '\n';
	out << "Portfolio Variance: " << portfolioVariance << '\n';
	out << "Portfolio Standard Deviation: " << portfolioSD << '\n';
	out << string(50, '-') << '\n';

	out << "Optimizing weights for minimum risk: " << '\n';
	out << "Optimal weights: ";
	vector<double> optimalWeights = optimizeWeights(covMatrix);
	double riskFree = 0.02;
	out << "Nvidia: " << optimalWeights[0] << ", AMD: " << optimalWeights[1] << ", Intel: " << optimalWeights[2] << '\n';
	out << "Portfolio Variance with optimal weights: " << computePortfolioVariance(optimalWeights, covMatrix) << '\n';
	out << "Portfolio Standard Deviation with optimal weights: " << computePortfolioSD(optimalWeights, covMatrix) << '\n';
	out << "Portfolio Sharpe Ratio with optimal weights: " << computeSharpe(optimalWeights, { computeMean(a), computeMean(b), computeMean(c) }, covMatrix, riskFree / 252) * sqrt(252) << '\n';
	out << string(50, '-') << '\n';

	// Optimize Sharpe Ratio
	double dailyRiskFree = riskFree / 252;
	vector<double> means = { computeMean(a), computeMean(b), computeMean(c) };
	vector<double> optimalSharpe = optimizeSharpe({ computeMean(a), computeMean(b), computeMean(c) }, covMatrix, riskFree / 252);
	out << "Optimizing weights for maximum sharpe: " << '\n';
	out << "Optimal sharpe: ";
	out << "Nvidia: " << optimalSharpe[0] << ", AMD: " << optimalSharpe[1] << ", Intel: " << optimalSharpe[2] << '\n';
	out << "Portfolio Variance with optimal sharpe: " << computePortfolioVariance(optimalSharpe, covMatrix) << '\n';
	out << "Portfolio Standard Deviation with optimal sharpe: " << computePortfolioSD(optimalSharpe, covMatrix) << '\n';
	out << "Portfolio Sharpe Ratio with optimal sharpe: " << computeSharpe(optimalSharpe, { computeMean(a), computeMean(b), computeMean(c) }, covMatrix, riskFree / 252) * sqrt(252) << '\n';
	out << string(50, '-') << '\n';

	// Equal weight
	vector<double> wEq = { 0.4, 0.4, 0.2 };
	writeRow(filename, period, "EqualWeight", wEq, portfolioMean(wEq, means), computePortfolioSD(wEq, covMatrix), computeSharpe(wEq, means, covMatrix, dailyRiskFree) * sqrt(252.0));

	// Min variance
	vector<double> wMin = optimizeWeights(covMatrix);
	writeRow(filename, period, "MinVariance", wMin, portfolioMean(wMin, means), computePortfolioSD(wMin, covMatrix), computeSharpe(wMin, means, covMatrix, dailyRiskFree) * sqrt(252.0));

	// Max Sharpe
	vector<double> wMax = optimizeSharpe(means, covMatrix, dailyRiskFree);
	writeRow(filename, period, "MaxSharpe", wMax, portfolioMean(wMax, means), computePortfolioSD(wMax, covMatrix), computeSharpe(wMax, means, covMatrix, dailyRiskFree) * sqrt(252.0));

	// Test:
	vector<double> wGrid = optimizeWeights(covMatrix);
	vector<double> wGrad = gradientDescend(covMatrix, 1000, 100.0);
	for (int i = 0; i < wGrid.size(); i++)
		out << wGrid[i] << " ";
	out << endl;
	for (int i = 0; i < wGrid.size(); i++)
		out << wGrad[i] << " ";
	out << endl;
}

int main() {
	// Set precision for output
	cout << setprecision(6) << fixed;

	// Prices vectors
	StockData nvdia = readStockData("D:/C_Project/Stock Market Analyzer/Copy of Analyzer/Data/nvda_us_d.csv");
	StockData amd = readStockData("D:/C_Project/Stock Market Analyzer/Copy of Analyzer/Data/amd_us_d.csv");
	StockData intel = readStockData("D:/C_Project/Stock Market Analyzer/Copy of Analyzer/Data/intc_us_d.csv");

	if (nvdia.prices.empty() || amd.prices.empty() || intel.prices.empty()) {
		cerr << "Error: Failed to read stock data." << endl;
		return 1;
	}

	cout << "Before alignment: " << '\n';
	cout << "Nvidia: " << nvdia.prices.size() << " prices\n";
	cout << "AMD: " << amd.prices.size() << " prices\n";
	cout << "Intel: " << intel.prices.size() << " prices\n";
	cout << string(50, '-') << '\n';

	dateAlign(nvdia, amd, intel, 1);

	// Returns vectors
	vector<double> nvdiaReturns = computeReturns(nvdia.prices);
	vector<double> amdReturns = computeReturns(amd.prices);
	vector<double> intelReturns = computeReturns(intel.prices);

	cout << "After alignment: " << nvdia.prices.size() << " common days\n";
	cout << "Range: " << nvdia.dates.front()
		<< " to " << nvdia.dates.back() << '\n';
	cout << string(50, '-') << '\n';

	const string csvFile = "results.csv";
	{
		ofstream header(csvFile);
		header << "period,strategy,nvda,amd,intc,mean,sd,sharpe\n";
	}   // file closes here, flushing the header

	output(cout, "Full", csvFile, nvdiaReturns, amdReturns, intelReturns);

	// Backtest
	size_t len = nvdia.prices.size();

	StockData nvdiaFirst = sliceStockData(nvdia, 0, len / 2);
	StockData amdFirst = sliceStockData(amd, 0, len / 2);
	StockData intelFirst = sliceStockData(intel, 0, len / 2);

	vector<double> nvdiaFirstReturns = computeReturns(nvdiaFirst.prices);
	vector<double> amdFirstReturns = computeReturns(amdFirst.prices);
	vector<double> intelFirstReturns = computeReturns(intelFirst.prices);

	output(cout, "FirstHalf", csvFile, nvdiaFirstReturns, amdFirstReturns, intelFirstReturns);

	StockData nvdiaSecond = sliceStockData(nvdia, len / 2, len - 1);
	StockData amdSecond = sliceStockData(amd, len / 2, len - 1);
	StockData intelSecond = sliceStockData(intel, len / 2, len - 1);

	vector<double> nvdiaSecondReturns = computeReturns(nvdiaSecond.prices);
	vector<double> amdSecondReturns = computeReturns(amdSecond.prices);
	vector<double> intelSecondReturns = computeReturns(intelSecond.prices);

	output(cout, "SecondHalf", csvFile, nvdiaSecondReturns, amdSecondReturns, intelSecondReturns);

	// Using first half weights to estimate second half
	vector<double> firstWeights = { 0.5, 0.06, 0.44 };
	vector<vector<double>> covSecond = computeCovMatrix({ nvdiaSecondReturns, amdSecondReturns, intelSecondReturns });
	vector<double> meansSecond = { computeMean(nvdiaSecondReturns),computeMean(amdSecondReturns),computeMean(intelSecondReturns) };

	cout << "Out-of-sample SD: "
		<< computePortfolioSD(firstWeights, covSecond) << '\n';
	cout << "Out-of-sample Sharpe: "
		<< computeSharpe(firstWeights, meansSecond, covSecond, 0.02 / 252.0) * sqrt(252.0) << '\n';

	return 0;
}