//
// Created by Bangtian Liu on 6/30/19.
//

#ifndef PROJECT_COMPRESSION_H
#define PROJECT_COMPRESSION_H

#include <vector>
#include <algorithm>
#include <random>
#include <cstring>
#include "../sympiler/HMatrix.h"
#include "HTree.h"
#include "../sympiler/HTree.h"


using namespace Sympiler::Internal;
using namespace Sympiler;

struct cretvalue {
	int *skels;
	int skels_length;
	double *proj;
	int proj_column;
};


struct DDcost {
	int index;
	unsigned long cost;
};

bool fcompare(DDcost lhs, DDcost rhs) {
	return lhs.cost > rhs.cost;
}

typedef cretvalue ret;

using namespace std;

void Fsubmatrix(std::vector<int> &amap, std::vector<int> &bmap, double *submatrix, double *X, int n, int d,
				Internal::Ktype ktype,
				double h) {
	switch (ktype) {
		case Internal::KS_GAUSSIAN: {
			double *source = (double *) mkl_malloc(sizeof(double) * bmap.size() * d, 64);
			double *target = (double *) mkl_malloc(sizeof(double) * amap.size() * d, 64);

#pragma omp parallel for
			for (int i = 0; i < bmap.size(); i++) {
				for (int j = 0; j < d; j++) {
					source[i * d + j] = X[bmap[i] * d + j];
				}
			}

#pragma omp parallel for
			for (int i = 0; i < amap.size(); i++) {
				for (int j = 0; j < d; j++) {
					target[i * d + j] = X[amap[i] * d + j];
				}
			}

			cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans,
						amap.size(), bmap.size(), d, -2.0,
						target, d,
						source, d, 0.0,
						submatrix, amap.size());

			double *target_sqnorms = (double *) mkl_malloc(sizeof(double) * amap.size(), 64);
			double *source_sqnorms = (double *) mkl_malloc(sizeof(double) * bmap.size(), 64);

#pragma omp parallel for
			for (int i = 0; i < amap.size(); i++) {
				target_sqnorms[i] = cblas_ddot(d,
											   target + i * d, 1,
											   target + i * d, 1);
			}

			for (int i = 0; i < bmap.size(); i++) {
				source_sqnorms[i] = cblas_ddot(d,
											   source + i * d, 1,
											   source + i * d, 1);
			}
#pragma omp parallel for
			for (int j = 0; j < bmap.size(); j++) {
				for (int i = 0; i < amap.size(); i++)
					submatrix[j * amap.size() + i] += target_sqnorms[i] + source_sqnorms[j];
			}
			double kscal = -0.5 / (h * h);
#pragma omp parallel for
			for (int i = 0; i < amap.size() * bmap.size(); i++) {
				submatrix[i] = std::exp(kscal * submatrix[i]);
			}

			break;
		}

		case Internal::KS_LOG: {
//#pragma omp parallel for
//			for (int i = 0; i < amap.size() * bmap.size(); i++) {
//				submatrix[i] = -0.5 * log(submatrix[i]);
//			}

			break;
		}

		case Internal::KS_EXPONENTIAL: {
//#pragma omp parallel for
//			for (int i = 0; i < amap.size() * bmap.size(); i++) {
//				submatrix[i] = exp(-sqrt(submatrix[i]));;
//			}
			break;
		}

		case Internal::KS_NEWTON: {
#pragma omp parallel for
			for (int j = 0; j < bmap.size(); j++) {
				for (int i = 0; i < amap.size(); i++) {
					auto Kij = 0.0;
					for (int k = 0; k < d; ++k) {
						auto col = bmap[j];
						auto row = amap[i];
						auto tar = X[col * d + k];
						auto src = X[row * d + k];
						Kij += (tar - src) * (tar - src);
					}
					if(Kij==0)Kij=1;
					submatrix[j * amap.size() + i] = 1/sqrt(Kij);
				}
			}
//#pragma omp parallel for
//			for (int i = 0; i < amap.size() * bmap.size(); i++) {
//				if(submatrix[i]==0) submatrix[i] = 1;
//				else submatrix[i] = 1/std::sqrt(submatrix[i]);
//			}

			break;
		}

		default: {
			printf("invalid kernel type\n");
			exit(1);
			break;
		}
	}

}


//
//void DDDFsubmatrix(std::vector<int> &amap, std::vector<int> &bmap, double *submatrix, double *X, int n, int d, Internal::Ktype ktype,
//				double h)
//{
//
//	double *source = (double *)mkl_malloc(sizeof(double)*bmap.size()*d, 64);
//	double *target = (double *)mkl_malloc(sizeof(double)*amap.size()*d, 64);
//
//#pragma omp parallel for
//	for(int i=0; i<bmap.size(); i++)
//	{
//		for(int j=0; j<d; j++)
//		{
//			source[i*d + j] = X[bmap[i]*d+j];
//		}
//	}
//
//#pragma omp parallel for
//	for(int i=0; i<amap.size(); i++) {
//		for (int j = 0; j < d; j++) {
//			target[i*d + j] = X[amap[i]*d+j];
//		}
//	}
//
//	cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans,
//				amap.size(), bmap.size(), d, -2.0,
//				target, d,
//				source, d, 0.0,
//				submatrix, amap.size());
//
//	double *target_sqnorms = (double *)mkl_malloc(sizeof(double)*amap.size(), 64);
//	double *source_sqnorms = (double *)mkl_malloc(sizeof(double)*bmap.size(), 64);
//
//#pragma omp parallel for
//	for(int i=0; i<amap.size(); i++)
//	{
//		target_sqnorms[i] = cblas_ddot(d,
//									   target+i*d, 1,
//									   target+i*d, 1);
//	}
//
//	for(int i=0; i<bmap.size(); i++)
//	{
//		source_sqnorms[i] = cblas_ddot(d,
//									   source+i*d,1,
//									   source+i*d,1);
//	}
//#pragma omp parallel for
//	for(int j=0; j<bmap.size(); j++)
//	{
//		for(int i=0; i<amap.size(); i++)
//			submatrix[j*amap.size() + i] += target_sqnorms[i] + source_sqnorms[j];
//	}
//
//	switch (ktype) {
//		case Internal::KS_GAUSSIAN: {
//
//			double kscal = -0.5/(h * h);
//#pragma omp parallel for
//			for(int i=0; i<amap.size()*bmap.size(); i++)
//			{
//				submatrix[i] = 1.0;
//			}
//
//			break;
//		}
//
//		case Internal::KS_LOG: {
//#pragma omp parallel for
//			for(int i=0; i<amap.size()*bmap.size(); i++)
//			{
//				submatrix[i] = -0.5 * log(submatrix[i]);
//			}
//
//			break;
//		}
//
//		case Internal::KS_EXPONENTIAL: {
//#pragma omp parallel for
//			for(int i=0; i<amap.size()*bmap.size(); i++)
//			{
//				submatrix[i] = exp(-sqrt(submatrix[i]));;
//			}
//			break;
//		}
//
//		case Internal::KS_NEWTON: {
//
//#pragma omp parallel for
//			for(int i=0; i<amap.size()*bmap.size(); i++)
//			{
//				submatrix[i] = std::sqrt(submatrix[i]);
//			}
//
//			break;
//		}
//
//		default: {
//			printf("invalid kernel type\n");
//			exit(1);
//			break;
//		}
//	}
//
//}

void Fsubmatrix(int *amap, int lena, int *bmap, int lenb, double *submatrix, Internal::Ktype ktype, double *X, int d,
				double h) {

	switch (ktype) {
		case Internal::KS_GAUSSIAN: {

			double *source = (double *) mkl_malloc(sizeof(double) * lenb * d, 64);
			double *target = (double *) mkl_malloc(sizeof(double) * lena * d, 64);

#pragma omp parallel for
			for (int i = 0; i < lenb; i++) {
				for (int j = 0; j < d; j++) {
					source[i * d + j] = X[bmap[i] * d + j];
				}
			}

#pragma omp parallel for
			for (int i = 0; i < lena; i++) {
				for (int j = 0; j < d; j++) {
					target[i * d + j] = X[amap[i] * d + j];
				}
			}

			cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans,
						lena, lenb, d, -2.0,
						target, d,
						source, d, 0.0,
						submatrix, lena);

			double *target_sqnorms = (double *) mkl_malloc(sizeof(double) * lena, 64);
			double *source_sqnorms = (double *) mkl_malloc(sizeof(double) * lenb, 64);

#pragma omp parallel for
			for (int i = 0; i < lena; i++) {
				target_sqnorms[i] = cblas_ddot(d,
											   target + i * d, 1,
											   target + i * d, 1);
			}
#pragma omp parallel for
			for (int i = 0; i < lenb; i++) {
				source_sqnorms[i] = cblas_ddot(d,
											   source + i * d, 1,
											   source + i * d, 1);
			}
#pragma omp parallel for
			for (int j = 0; j < lenb; j++) {
				for (int i = 0; i < lena; i++)
					submatrix[j * lena + i] += target_sqnorms[i] + source_sqnorms[j];
			}




			double kscal = -0.5 / (h * h);
#pragma omp parallel for
			for (int i = 0; i < lena * lenb; i++) {
				submatrix[i] = std::exp(kscal * submatrix[i]);
			}

			break;
		}

		case Internal::KS_LOG: {
//#pragma omp parallel for
//			for (int i = 0; i < lena * lenb; i++) {
//				submatrix[i] = -0.5 * log(submatrix[i]);
//			}

			break;
		}

		case Internal::KS_EXPONENTIAL: {
//#pragma omp parallel for
//			for (int i = 0; i < lena * lenb; i++) {
//				submatrix[i] = exp(-sqrt(submatrix[i]));;
//			}
//			break;
		}

		case Internal::KS_NEWTON: {
#pragma omp parallel for
			for (int j = 0; j < lenb; j++) {
				for (int i = 0; i < lena; i++) {
					auto Kij = 0.0;
					for (int k = 0; k < d; ++k) {
						auto col = bmap[j];
						auto row = amap[i];
						auto tar = X[col * d + k];
						auto src = X[row * d + k];
						Kij += (tar - src) * (tar - src);
					}
					if(Kij==0)Kij=1;
					submatrix[j * lena + i] = 1/sqrt(Kij);
				}
			}
			break;
		}

		default: {
			printf("invalid kernel type\n");
			exit(1);
			break;
		}
	}
}

void BuildNeighBorsLeaf(HTree &tree, int idx, int nsamples) {
	auto &pnids = tree.pnids[idx];
	auto &snids = tree.snids[idx];
	int k = 32;
	auto &NN = tree.NN;
	auto lids = tree.lids + tree.lidsoffset[idx];
	int n = tree.lidslen[idx];
//	printf("len=%d\n",n);
	pnids = std::unordered_set<int>(); // will modify

	for (int ii = 0; ii < k / 2; ii++) {
		for (int jj = 0; jj < n; jj++) {
			auto idx = NN[lids[jj] * k + ii].second;
			pnids.insert(idx);
//						printf("%lu;",NN[ lids[jj] * k + ii].second);
		}
	}
	for (int i = 0; i < n; i++) {
		pnids.erase(lids[i]);
	}
//		printf("Leaf Size of pruning neighbor set: %lu \n", pnids.size());
	snids = std::map<int, double>();
	std::vector<std::pair<double, int>> tmp(k / 2 * n);
	std::set<int> nodeIdx(lids, lids + n);
	// Allocate array for sorting
	for (int ii = (k + 1) / 2; ii < k; ii++) {
		for (int jj = 0; jj < n; jj++) {
			tmp[(ii - (k + 1) / 2) * n + jj] = NN[lids[jj] * k + ii];
		}
	}

	std::sort(tmp.begin(), tmp.end());
	int i = 0;

	while (snids.size() < nsamples && i < (k - 1) * n / 2) {
		if (!pnids.count(tmp[i].second) && !nodeIdx.count(tmp[i].second)) {
			snids.insert(std::pair<int, double>(tmp[i].second, tmp[i].first));
		}
		i++;
	}
}


void BuildNeighBorsInternal(HTree &tree, int idx, int nsamples) {
	auto &pnids = tree.pnids[idx];
	auto &snids = tree.snids[idx];
	int k = 32;
	auto &NN = tree.NN;
	auto lids = tree.lids + tree.lidsoffset[idx];
	int n = tree.lidslen[idx];

	auto &lsnids = tree.snids[tree.tlchildren[idx]];
	auto &rsnids = tree.snids[tree.trchildren[idx]];

	auto &lpnids = tree.pnids[tree.tlchildren[idx]];
	auto &rpnids = tree.pnids[tree.trchildren[idx]];

	snids = lsnids;

	for (auto cur = rsnids.begin(); cur != rsnids.end(); cur++) {
		auto ret = snids.insert(*cur);
		if (ret.second == false) {
			// Update distance?
			if (ret.first->second > (*cur).first) {
				ret.first->second = (*cur).first;
			}
		}
	}

	// Remove "own" points
	for (int i = 0; i < n; i++) {
		snids.erase(lids[i]);
	}

	// Remove pruning neighbors from left and right
	for (auto cur = lpnids.begin(); cur != lpnids.end(); cur++) {
		snids.erase(*cur);
	}
	for (auto cur = rpnids.begin(); cur != rpnids.end(); cur++) {
		snids.erase(*cur);
	}
}


int decomposition(double *A, int nRows, int nCols, double tolerance, int **skels, double **proj, int **jpvt) {
	assert(nRows > nCols);
	int s;
	int maxRank = 256;
//    printf("maxRank=%d\n",maxRank);
	*jpvt = (int *) malloc(sizeof(int) * nCols);
	memset(*jpvt, 0, sizeof(int) * nCols);
//    T *tau = GenMatrix<T>(std::min(nRows,nCols),1);
	double *tau = (double *) mkl_malloc(sizeof(double) * std::min(nRows, nCols), 64);

	auto info = LAPACKE_dgeqp3(LAPACK_COL_MAJOR, nRows, nCols, A, nRows, *jpvt, tau);

	if (info != 0) {
		printf("%d-th parameter had an illegal value", -info);
	}

#pragma omp parallel for
	for (int i = 0; i < nCols; ++i) {
		(*jpvt)[i] = (*jpvt)[i] - 1;
	}

	for (s = 1; s < nCols; ++s) {
//        printf("s=%d, a=%e, error=%e nCOls=%d\n",s, A[s*nRows+s],tolerance,nCols);
		if (s > maxRank || std::abs(A[s * nRows + s]/A[0]) < tolerance) break;
	}


//	if(!setup.adaptive)
//	{
//		s = std::min(maxRank, nCols);
//	}

	if (s > maxRank) s = maxRank;
	*skels = (int *) malloc(sizeof(int) * s);
	memcpy(*skels, *jpvt, sizeof(int) * s);
//    memcpy(*skels,*proj, sizeof(int)*s);

//    *proj = GenMatrix<T>(s,nCols);
	*proj = (double *) mkl_malloc(sizeof(double) * s * nCols, 64);
	memset(*proj, 0, sizeof(double) * s * nCols);

//#pragma omp parallel for
	for (int j = 0; j < nCols; j++) {
		for (int i = 0; i < s; i++) {
			if (j < s) {
				if (j >= i) (*proj)[j * s + i] = A[j * nRows + i];
				else (*proj)[j * s + i] = 0.0;
			} else {
				(*proj)[j * s + i] = A[j * nRows + i];
			}
		}
	}

	if ((*proj)[0] == 0) return s;  // put on here
	double *R1 = (double *) mkl_malloc(sizeof(double) * s * s, 64);
	memset(R1, 0, sizeof(double) * s * s);  // todo check the segment fault bug here

//#pragma omp parallel for
	for (int j = 0; j < s; j++) {
		for (int i = 0; i < s; i++) {
			if (i <= j) R1[j * s + i] = (*proj)[j * s + i];
//            if((*proj)[j*s+i]!=(*proj)[j*s+i])printf("NAN FOUND1!!!\n");
		}
	}
//    T *tmp = GenMatrix<T>(s,nCols);
	double *tmp = (double *) mkl_malloc(sizeof(double) * s * nCols, 64);
	memcpy(tmp, *proj, sizeof(double) * s * nCols);

	cblas_dtrsm(CblasColMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, s, nCols, 1.0, R1, s, tmp, s);

	/** Fill in proj */
	for (int j = 0; j < nCols; j++) {
		for (int i = 0; i < s; i++) {
			(*proj)[(*jpvt)[j] * s + i] = tmp[j * s + i];
		}
	}
	return s;
}


void skeletonize_leaf(int idx, int *lids, int *lidslen, int *lidsoffset, ret *rtmp, int *sid, int *sidlen,
					  int *sidoffset, int *lc, int *rc, int m, int n, double *X, Internal::Ktype ktype, int dim,
					  double h,
					  double acc) {
	std::vector<int> bmap;
	bmap.insert(bmap.end(), lids + lidsoffset[idx], lids + lidsoffset[idx] + lidslen[idx]);

	auto nsamples = 2 * bmap.size();
	auto numpoints = lidslen[idx];
	auto clids = lids + lidsoffset[idx];

	nsamples = (nsamples < 2 * m) ? 2 * m : nsamples;

	int slen = sidlen[idx];
	int offset = sidoffset[idx];
//	printf("idx =%d slen=%d\n", idx, slen);
	// add sampling points
	std::vector<int> amap;
//			(sid+offset, sid+offset+slen);

	mt19937 generator(idx);
	uniform_int_distribution<> uniform_distribution(0, n - 1);
	if (nsamples < (n - numpoints)) {
		amap.assign(sid + offset, sid + offset + slen);
		while (amap.size() < nsamples) {
//					auto sample = rand() % setup.n;
			auto sample = uniform_distribution(generator);
			if (std::find(amap.begin(), amap.end(), sample) == amap.end() &&
				std::find(clids, clids + numpoints, sample) == (clids + numpoints)) {
				amap.push_back(sample);
			}
		}
	} else {
		for (int sample = 0; sample < n; sample++)  // TODO: may can be improved here
		{
			if (std::find(amap.begin(), amap.end(), sample) == amap.end()) {
				amap.push_back(sample);
			}
		}
	}

	auto Kab = (double *) malloc(sizeof(double) * amap.size() * bmap.size());
	memset(Kab, 0, sizeof(double) * amap.size() * bmap.size());

	Fsubmatrix(amap, bmap, Kab, X, n, dim, ktype, h);

	auto N = n;
	auto m1 = amap.size();
	auto n1 = bmap.size();
	auto q = numpoints;
//	std::sqrt((m1 * n1 * 1.0) / (1.0 * N * (N - q))) *
	auto tolerance = acc;

	int *skels;
	double *proj;
	int *jpvt;

	int s = decomposition(Kab, m1, n1, tolerance, &skels, &proj, &jpvt);

	for (int i = 0; i < s; ++i) {  // need to check it
		skels[i] = bmap[skels[i]];
	}

	rtmp[idx].skels = skels;
	rtmp[idx].skels_length = s;
	rtmp[idx].proj = proj;
	rtmp[idx].proj_column = (int) bmap.size();
	free(Kab);

}

void skeletonize_leaf(int idx, HTree &tree, ret *rtmp, int m, int n, double *X, Internal::Ktype ktype, int dim,
					  double h, double acc) {
	auto lc = tree.tlchildren;
	auto rc = tree.trchildren;
	auto lids = tree.lids;
	auto lidslen = tree.lidslen;
	auto lidsoffset = tree.lidsoffset;


	std::vector<int> bmap;
	bmap.insert(bmap.end(), lids + lidsoffset[idx], lids + lidsoffset[idx] + lidslen[idx]);

	auto nsamples = 2 * bmap.size();
	auto numpoints = lidslen[idx];
	auto clids = lids + lidsoffset[idx];

	nsamples = (nsamples < 2 * m) ? 2 * m : nsamples;

	BuildNeighBorsLeaf(tree, idx, nsamples);

	auto &snids = tree.snids[idx];
	auto &pnids = tree.pnids[idx];

	std::multimap<double, int> ordered_snids = flip_map(snids);

	// add sampling points
	std::vector<int> amap;
//			(sid+offset, sid+offset+slen);

//	mt19937 generator(idx);
//	uniform_int_distribution<> uniform_distribution(0, n - 1);
//	printf("idx =%d, len=%ld %ld\n", idx, ordered_snids.size(), nsamples);
	if (nsamples < (n - numpoints)) {
		amap.reserve(nsamples);
		for (auto cur = ordered_snids.begin(); cur != ordered_snids.end(); cur++) {
			amap.push_back(cur->second);
		}
		while (amap.size() < nsamples) {
					auto sample = rand() % n;
//			auto sample = uniform_distribution(generator);
			if (std::find(amap.begin(), amap.end(), sample) == amap.end() &&
				std::find(clids, clids + numpoints, sample) == (clids + numpoints)) {
				amap.push_back(sample);
			}
		}
	} else {
		for (int sample = 0; sample < n; sample++)  // TODO: may can be improved here
		{
			if (std::find(amap.begin(), amap.end(), sample) == amap.end()) {
				amap.push_back(sample);
			}
		}
	}

	auto Kab = (double *) malloc(sizeof(double) * amap.size() * bmap.size());
	memset(Kab, 0, sizeof(double) * amap.size() * bmap.size());

	Fsubmatrix(amap, bmap, Kab, X, n, dim, ktype, h);

	auto N = n;
	auto m1 = amap.size();
	auto n1 = bmap.size();
	auto q = numpoints;


//	double scal_tol = std::sqrt((double)n1/q) *std::sqrt((double)m1/(N-q)) * acc;

//	auto tolerance = std::sqrt((double)q/N) * scal_tol;

	auto tolerance = acc;
	int *skels;
	double *proj;
	int *jpvt;

	int s = decomposition(Kab, m1, n1, tolerance, &skels, &proj, &jpvt);

	for (int i = 0; i < s; ++i) {  // need to check it
		skels[i] = bmap[skels[i]];
	}


	pnids.clear();
	auto &NN = tree.NN;
	for (int ii = 0; ii < s; ii++) {
		for (int jj = 0; jj < 32 / 2; jj++) {
			pnids.insert(NN.data()[skels[ii] * 32 + jj].second);
		}
	}

	rtmp[idx].skels = skels;
	rtmp[idx].skels_length = s;
	rtmp[idx].proj = proj;
	rtmp[idx].proj_column = (int) bmap.size();
	free(Kab);
}

void skeletonize_internal(int idx, HTree &tree, ret *rtmp, int m, int n, double *X, Internal::Ktype ktype, int dim,
						  double h, double acc) {
	auto lc = tree.tlchildren;
	auto rc = tree.trchildren;
	auto lids = tree.lids;
	auto lidslen = tree.lidslen;
	auto lidsoffset = tree.lidsoffset;


	std::vector<int> bmap;
	auto v = lc[idx];
	bmap.insert(bmap.end(), rtmp[v].skels, rtmp[v].skels + rtmp[v].skels_length);
	v = rc[idx];
	bmap.insert(bmap.end(), rtmp[v].skels, rtmp[v].skels + rtmp[v].skels_length);

	auto nsamples = 2 * bmap.size();
	auto numpoints = lidslen[idx];
	auto clids = lids + lidsoffset[idx];

	nsamples = (nsamples < 2 * m) ? 2 * m : nsamples;

	BuildNeighBorsInternal(tree, idx, nsamples);

	auto &snids = tree.snids[idx];
	auto &pnids = tree.pnids[idx];

	std::multimap<double, int> ordered_snids = flip_map(snids);
	std::vector<int> amap;
//			(sid+offset, sid+offset+slen);
//	mt19937 generator(idx);
//	uniform_int_distribution<> uniform_distribution(0, n - 1);
//	printf("idx=%d len=%ld %ld\n", idx, ordered_snids.size(), nsamples);
	if (nsamples < (n - numpoints)) {
//		printf("idx=%d, len=%d nsamples=%d\n", idx, ordered_snids.size(), nsamples);
		amap.reserve(nsamples);
    if(ordered_snids.size()>nsamples){
       auto cur = ordered_snids.begin();
       for(int k=0; k<nsamples; k++)
       {
         amap.push_back(cur->second);
         cur++;
       }
    }
    else {
		    for (auto cur = ordered_snids.begin(); cur != ordered_snids.end(); cur++) {
			      amap.push_back(cur->second);}
    }
		
    while (amap.size() < nsamples) {
					auto sample = rand() % n;
//			auto sample = uniform_distribution(generator);
			if (std::find(amap.begin(), amap.end(), sample) == amap.end() &&
				std::find(clids, clids + numpoints, sample) == (clids + numpoints)) {
				amap.push_back(sample);
			}
		}
	} else {
		for (int sample = 0; sample < n; sample++)  // TODO: may can be improved here
		{
			if (std::find(amap.begin(), amap.end(), sample) == amap.end()) {
				amap.push_back(sample);
			}
		}
	}

	auto Kab = (double *) malloc(sizeof(double) * amap.size() * bmap.size());
	memset(Kab, 0, sizeof(double) * amap.size() * bmap.size());

	Fsubmatrix(amap, bmap, Kab, X, n, dim, ktype, h);

	auto N = n;
	auto m1 = amap.size();
	auto n1 = bmap.size();
	auto q = numpoints;

//	double scal_tol = std::sqrt((double)n1/q) *std::sqrt((double)m1/(N-q)) * acc;
//
//	auto tolerance = std::sqrt((double)q/N) * scal_tol;

	auto tolerance = acc;

	int *skels;
	double *proj;
	int *jpvt;

	int s = decomposition(Kab, m1, n1, tolerance, &skels, &proj, &jpvt);

	for (int i = 0; i < s; ++i) {  // need to check it
		skels[i] = bmap[skels[i]];
	}

	pnids.clear();
	auto &NN = tree.NN;
	for (int ii = 0; ii < s; ii++) {
		for (int jj = 0; jj < 32 / 2; jj++) {
			pnids.insert(NN.data()[skels[ii] * 32 + jj].second);
		}
	}

	rtmp[idx].skels = skels;
	rtmp[idx].skels_length = s;
	rtmp[idx].proj = proj;
	rtmp[idx].proj_column = (int) bmap.size();
//	free(Kab);
}


void skeletonize_internal(int idx, int *lids, int *lidslen, int *lidsoffset, ret *rtmp, int *sid, int *sidlen,
						  int *sidoffset, int *lc, int *rc, int m, int n, double *X, Internal::Ktype ktype, int dim,
						  double h, double acc) {
	std::vector<int> bmap;
	auto v = lc[idx];
	bmap.insert(bmap.end(), rtmp[v].skels, rtmp[v].skels + rtmp[v].skels_length);
	v = rc[idx];
	bmap.insert(bmap.end(), rtmp[v].skels, rtmp[v].skels + rtmp[v].skels_length);

	auto nsamples = 2 * bmap.size();
	auto numpoints = lidslen[idx];
	auto clids = lids + lidsoffset[idx];

	nsamples = (nsamples < 2 * m) ? 2 * m : nsamples;

	int slen = sidlen[idx];
	int offset = sidoffset[idx];
	std::vector<int> amap;

	mt19937 generator(idx);
	uniform_int_distribution<> uniform_distribution(0, n - 1);
	if (nsamples < (n - numpoints)) {
//		printf("idx=%d, len=%d nsamples=%d\n", idx, slen, nsamples);
		amap.assign(sid + offset, sid + offset + slen);
		while (amap.size() < nsamples) {
//					auto sample = rand() % setup.n;
			auto sample = uniform_distribution(generator);
			if (std::find(amap.begin(), amap.end(), sample) == amap.end() &&
				std::find(clids, clids + numpoints, sample) == (clids + numpoints)) {
				amap.push_back(sample);
			}
		}
	} else {
		for (int sample = 0; sample < n; sample++)  // TODO: may can be improved here
		{
			if (std::find(amap.begin(), amap.end(), sample) == amap.end()) {
				amap.push_back(sample);
			}
		}
	}

	auto Kab = (double *) malloc(sizeof(double) * amap.size() * bmap.size());
	memset(Kab, 0, sizeof(double) * amap.size() * bmap.size());

	Fsubmatrix(amap, bmap, Kab, X, n, dim, ktype, h);

	auto N = n;
	auto m1 = amap.size();
	auto n1 = bmap.size();
	auto q = numpoints;

	auto tolerance = std::sqrt((m1 * n1 * 1.0) / (1.0 * N * (N - q))) * acc;

	int *skels;
	double *proj;
	int *jpvt;

	int s = decomposition(Kab, m1, n1, tolerance, &skels, &proj, &jpvt);

	for (int i = 0; i < s; ++i) {  // need to check it
		skels[i] = bmap[skels[i]];
	}

	rtmp[idx].skels = skels;
	rtmp[idx].skels_length = s;
	rtmp[idx].proj = proj;
	rtmp[idx].proj_column = (int) bmap.size();
	free(Kab);
}

void skeletonize(int idx, int *lids, int *lidslen, int *lidsoffset, ret *rtmp, int *sid, int *sidlen,
				 int *sidoffset, int *lc, int *rc, int m, int n, double *X, Internal::Ktype ktype, int dim, double h) {

//	if(idx==0){
//		return;
//	}

	std::vector<int> bmap;
	if (lc[idx] == -1) { // leaf node
		bmap.insert(bmap.end(), lids + lidsoffset[idx], lids + lidsoffset[idx] + lidslen[idx]);
	} else {
		auto v = lc[idx];
		bmap.insert(bmap.end(), rtmp[v].skels, rtmp[v].skels + rtmp[v].skels_length);
		v = rc[idx];
		bmap.insert(bmap.end(), rtmp[v].skels, rtmp[v].skels + rtmp[v].skels_length);
	}

//	printf("bmap size=%d len=%d\n",bmap.size(), lidslen[idx]);

	auto nsamples = 2 * bmap.size();
	auto numpoints = lidslen[idx];
	auto clids = lids + lidsoffset[idx];

	nsamples = (nsamples < 2 * m) ? 2 * m : nsamples;

	int slen = sidlen[idx];
	int offset = sidoffset[idx];

	// add sampling points
	std::vector<int> amap(sid + offset, sid + offset + slen);

	mt19937 generator(idx);
	uniform_int_distribution<> uniform_distribution(0, n - 1);
	if (nsamples < (n - numpoints)) {
		while (amap.size() < nsamples) {
//					auto sample = rand() % setup.n;
			auto sample = uniform_distribution(generator);
			if (std::find(amap.begin(), amap.end(), sample) == amap.end() &&
				std::find(clids, clids + numpoints, sample) == (clids + numpoints)) {
				amap.push_back(sample);
			}
		}
	} else {
		for (int sample = 0; sample < n; sample++)  // TODO: may can be improved here
		{
			if (std::find(amap.begin(), amap.end(), sample) == amap.end()) {
				amap.push_back(sample);
			}
		}
	}

	auto Kab = (double *) malloc(sizeof(double) * amap.size() * bmap.size());
	memset(Kab, 0, sizeof(double) * amap.size() * bmap.size());

	Fsubmatrix(amap, bmap, Kab, X, n, dim, ktype, h);

	auto N = n;
	auto m1 = amap.size();
	auto n1 = bmap.size();
	auto q = numpoints;

	double error = 1e-5;
//	printf("m1=%d, n1=%d N=%d q=%d\n", m1, n1, N, q);
	auto tolerance = std::sqrt((m1 * n1 * 1.0) / (1.0 * N * (N - q))) * error;
//	printf("error=%lf\n", std::sqrt((m1*n1*1.0)/(1.0*N*(N-q))));
//	auto tolerance = 1E-5;
	int *skels;
	double *proj;
	int *jpvt;

	int s = decomposition(Kab, m1, n1, tolerance, &skels, &proj, &jpvt);
//	printf("s=%d\n",s);
	for (int i = 0; i < s; ++i) {  // need to check it
		skels[i] = bmap[skels[i]];
	}

	rtmp[idx].skels = skels;
	rtmp[idx].skels_length = s;
	rtmp[idx].proj = proj;
	rtmp[idx].proj_column = (int) bmap.size();
	free(Kab);
}

void binpacking(std::vector<std::vector<int>> &wpartitions, std::vector<std::vector<int>> &owpartitions, int numofbins,
				clustertree &ctree, HTree &tree, ret *rtmp) {
	DDcost *ccost = new DDcost[wpartitions.size()];

	for (auto i = 0; i < wpartitions.size(); i++) {
		ccost[i].cost = 0;
		ccost[i].index = i;
		for (auto j = 0; j < wpartitions[i].size(); j++) {
			auto idx = wpartitions[i][j];
			unsigned long cost = 0;
			if (tree.tlchildren[idx] == -1) {
				cost += 2 * rtmp[idx].skels_length * rtmp[idx].proj_column;
//						leafdim[leafmap.at(idx)] * setup.nrhs;
			} else {
				auto lc = tree.tlchildren[idx];
				auto rc = tree.trchildren[idx];
				cost += 2 * rtmp[idx].skels_length * rtmp[lc].skels_length;
				cost += 2 * rtmp[idx].skels_length * rtmp[rc].skels_length;
//
//				for(auto &v : children[idx])
//				{
//					cost += 2*tmpresult[idx].skels_length*tmpresult[v].skels_length*setup.nrhs;
//				}
			}
			ccost[i].cost += cost;
		}
	}

	std::sort(ccost, ccost + wpartitions.size(), fcompare);

	uint64_t *ocost = new uint64_t[numofbins];

	memset(ocost, 0, sizeof(uint64_t) * numofbins);

	int partNo = wpartitions.size();

	int minBin = 0;
	for (int i = 0; i < partNo; i++) {
		minBin = findMin(ocost, numofbins);
		ocost[minBin] += ccost[i].cost;
		int index = ccost[i].index;
//owpartition

		owpartitions[minBin].insert(owpartitions[minBin].end(),
									wpartitions[index].begin(), wpartitions[index].end());
	}

}


void BalanceCoarLevelSet(clustertree &ctree, HTree &tree, ret *rtmp) {
	auto &postw = ctree.postw;
	auto &opostw = ctree.opostw;
	auto len = postw.size();
	opostw.resize(len);

//       		auto &pow=tree->postw;
//       		auto &opow=tree->opostw;
	for (int i = 0; i < postw.size(); i++) {
		auto &lpow = postw[i];
		int nw = lpow.size();

		int nparts;

		int nthreads = omp_get_max_threads();

		if (nw >= nthreads) {
			nparts = nthreads;
		} else {
			nparts = nw / 2;
		}
		if (nparts == 0)nparts = 1;
		opostw[i].resize(nparts);

		binpacking(lpow, opostw[i], nparts, ctree, tree, rtmp);
	}

	len = opostw.size();
	len = 0;
	int index = 0;
	for (auto &v:postw) {
		tree.clevelset[index++] = len;
		len += v.size();
	}
	tree.clevelset[index] = len;

	len = 0;
	index = 0;
	int tidx = 0;
	for (auto &v:postw) {
		for (auto &w:v) {
			tree.wpart[index++] = len;
			len += w.size();
			for (auto &t:w) {
				tree.idx[tidx++] = t;
			}
		}
	}
	tree.wpart[index] = len;

}



void compression(clustertree &ctree, HTree &tree, ret *rtmp, double *X, int m, int n, Internal::Ktype ktype, int dim, double h,
				 bool coarsening, double acc = 1e-5) {
#pragma omp parallel for
	for (int j = tree.levelset[tree.depth - 1]; j < tree.levelset[tree.depth]; j++) {
		auto id = tree.idx[j];
		skeletonize_leaf(id, tree, rtmp, m, n, X, ktype, dim, h, acc);
	}

	for (int i = tree.depth - 2; i > -1; i--) {
#pragma omp parallel for
		for (int j = tree.levelset[i]; j < tree.levelset[i + 1]; j++) {
			auto id = tree.idx[j];

			skeletonize_internal(id, tree, rtmp, m, n, X, ktype, dim, h, acc);

		}
	}
	if(coarsening)BalanceCoarLevelSet(ctree,tree,rtmp);
}


unsigned long computeFlops(HTree &tree, ret *rtmp, int m, int nrhs)
{
	unsigned long flops = 0;
	unsigned long tflops = 0;
  unsigned long inflops = 0;
  for (int j = tree.levelset[tree.depth - 1]; j < tree.levelset[tree.depth]; j++) {
		auto id = tree.idx[j];
		flops+=2*rtmp[id].skels_length * nrhs*tree.Dim[tree.lm[id]];
	}

	for (int i = tree.depth - 2; i > -1; i--) {
		for (int j = tree.levelset[i]; j < tree.levelset[i + 1]; j++)
		{
			auto id = tree.idx[j];
			auto lc = tree.tlchildren[id];
			auto rc = tree.trchildren[id];
			flops += 2 * rtmp[id].skels_length * (rtmp[lc].skels_length+rtmp[rc].skels_length)*nrhs;
		}
	}

	flops = 2*flops;

  tflops += flops;
//  printf("tree flops is %lu\n", tflops);
	for(int k=0; k<tree.ncount; k++)
	{
		auto nx = tree.nxval[k];
		auto ny = tree.nyval[k];

		auto dimx = tree.Dim[nx];
		auto dimy = tree.Dim[ny];

		flops += 2*dimx*dimy*nrhs;
	}
  
//  printf("number of near nodes is %d\n", tree.ncount);
  
	for(int k = 0; k<tree.fcount; k++)
	{
		auto fx = tree.fxval[k];
		auto fy = tree.fyval[k];

		auto dimx = rtmp[fx].skels_length;
		auto dimy = rtmp[fy].skels_length;

		flops += 2*dimx*dimy*nrhs;
	}
//  printf("number of far nodes is %d\n", tree.fcount);

//  printf("interaction flops is %lu\n", flops-tflops);
	return flops;
}


unsigned long computeRanks(HTree &tree, ret *rtmp, int m, int nrhs)
{
	unsigned long rank = 0;
	int count=0;
	for (int j = tree.levelset[tree.depth - 1]; j < tree.levelset[tree.depth]; j++) {
		auto id = tree.idx[j];
		rank +=rtmp[id].skels_length;
		++count;
		//		flops+=2*rtmp[id].skels_length * nrhs*tree.Dim[tree.lm[id]];
	}

	for (int i = tree.depth - 2; i > -1; i--) {
		for (int j = tree.levelset[i]; j < tree.levelset[i + 1]; j++)
		{
			auto id = tree.idx[j];
			rank +=rtmp[id].skels_length;
			++count;
//			auto lc = tree.tlchildren[id];
//			auto rc = tree.trchildren[id];
//			flops += 2 * rtmp[id].skels_length * (rtmp[lc].skels_length+rtmp[rc].skels_length)*nrhs;
		}
	}

	return rank/count;
}


void
coarcompression(HTree &tree, ret *rtmp, double *X, int m, int n, Internal::Ktype ktype, int dim, int h, double acc) {
#pragma omp parallel for
	for (int k = tree.clevelset[0]; k < tree.clevelset[1]; k++) {

		for (int j = tree.wpart[k]; j < tree.wpart[k + 1]; j++) {
			auto id = tree.cidx[j];
			if (tree.tlchildren[id] == -1) {
				skeletonize_leaf(id, tree.lids, tree.lidslen, tree.lidsoffset, rtmp, tree.sids, tree.sidlen,
								 tree.sidoffset, tree.tlchildren,
								 tree.trchildren, m, n, X, ktype, dim, h, acc);
			} else {
				skeletonize_internal(id, tree.lids, tree.lidslen, tree.lidsoffset, rtmp, tree.sids, tree.sidlen,
									 tree.sidoffset, tree.tlchildren,
									 tree.trchildren, m, n, X, ktype, dim, h, acc);
			}
//			skeletonize(id, tree.lids, tree.lidslen, tree.lidsoffset, rtmp, tree.sids, tree.sidlen, tree.sidoffset, tree.tlchildren,
//						tree.trchildren, m, n, X, ktype, dim, h);
		}
	}

	for (int i = 1; i < tree.cdepth; i++) {
#pragma omp parallel for
		for (int k = tree.clevelset[i]; k < tree.clevelset[i + 1]; k++) {
//#pragma omp parallel for
			for (int j = tree.wpart[k]; j < tree.wpart[k + 1]; j++) {
				auto id = tree.cidx[j];
				skeletonize_internal(id, tree.lids, tree.lidslen, tree.lidsoffset, rtmp, tree.sids, tree.sidlen,
									 tree.sidoffset, tree.tlchildren,
									 tree.trchildren, m, n, X, ktype, dim, h, acc);
			}
		}
	}
}







#endif //PROJECT_COMPRESSION_H
