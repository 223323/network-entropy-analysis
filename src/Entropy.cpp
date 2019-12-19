#include "Entropy.h"
#include <cmath>


// Entropy
// ----------------------------------------
Entropy::Entropy(double q) : m_value(0.0), m_count(0), m_q(q) {}
void Entropy::Add(double p) {
	
}

void Entropy::SetCount(uint32_t count) {
	m_count = count;
}
void Entropy::SetQ(double q) {
	m_q = q;
}
double Entropy::GetValue(bool normalized) {
	return m_value;
}

Entropy* Entropy::New() {
	return new Entropy(m_q);
}

// Shannon
// ----------------------------------------
ShannonEntropy::ShannonEntropy(double Q) : Entropy(Q) {
	
}

void ShannonEntropy::Add(double p) {
	if(p == 0) return;
	m_value += p * (-log2(p));
	m_count++;
}

double ShannonEntropy::GetValue(bool normalized) {
	return m_value * ((m_count > 1 && normalized) ? ( 100.0 / log2((double)m_count) ) : 1);
}

Entropy* ShannonEntropy::New() {
	return new ShannonEntropy(m_q);
}

// Tsalis
// ----------------------------------------
TsalisEntropy::TsalisEntropy(double q) : Entropy(q) {
}

void TsalisEntropy::Add(double p) {
	m_value += pow(p, m_q);
	m_count++;
}

double TsalisEntropy::GetValue(bool normalized) {
	if(normalized) {
		double denom = 1 - pow(m_count, 1-m_q);
		return (1 - m_value) * (denom != 0 ? (100.0 / denom) : 0);
	} else {
		return m_value;
	}
}

Entropy* TsalisEntropy::New() {
	return new TsalisEntropy(m_q);
}

// Tsalis2
// ----------------------------------------
Tsalis2Entropy::Tsalis2Entropy(double q) : Entropy(q) {
}

void Tsalis2Entropy::Add(double p) {
	m_value += pow(p, m_q);
	m_count++;
}

double Tsalis2Entropy::GetValue(bool normalized) {
	if(normalized) {
		double denom = 1-pow(m_count, 1-m_q);
		return std::abs( m_value * (denom != 0 ? (100.0 / denom) : 0) );
	} else {
		return m_value;
	}
}

Entropy* Tsalis2Entropy::New() {
	return new Tsalis2Entropy(m_q);
}

// Renyi entropy
// ----------------------------------------
RenyiEntropy::RenyiEntropy(double Q) : Entropy(Q) {
}

void RenyiEntropy::Add(double p) {
	m_value += pow(p, m_q);
	m_count++;
}

double RenyiEntropy::GetValue(bool normalized) {
	if (m_value == 0) return 0;
	double denom = log(pow(m_count, (1 - m_q)));
	return std::abs( log(m_value) * (denom != 0 ? (100.0 / denom) : 0) );
}

Entropy* RenyiEntropy::New() {
	return new RenyiEntropy(m_q);
}

// Renyi2 entropy
// ----------------------------------------
/*
Renyi2Entropy::Renyi2Entropy(double Q) : Entropy(Q) {
}

void Renyi2Entropy::Add(double p) {
	m_value += pow(p, m_q);
	m_count++;
}

double Renyi2Entropy::GetValue(bool normalized) {
	if (m_value == 0) return 0;
	// return log(m_value) / (1-m_q);
	return m_value * 100.0 / (log(pow(m_count,(1-m_q))));
}

Entropy* Renyi2Entropy::New() {
	return new Renyi2Entropy(m_q);
}
*/

// Ubriaco entropy
// ----------------------------------------
UbriacoEntropy::UbriacoEntropy(double Q) : Entropy(Q) {
}

void UbriacoEntropy::Add(double p) {
	if(p == 0) return;
	m_value += pow(-log(p), m_q) * p;
	m_count++;
}

double UbriacoEntropy::GetValue(bool normalized) {
	return m_value * 100 / pow(log((double)m_count), m_q);
}

Entropy* UbriacoEntropy::New() {
	return new UbriacoEntropy(m_q);
}

// BhatiaSingh entropy
// ----------------------------------------
BhatiaSinghEntropy::BhatiaSinghEntropy(double Q) : Entropy(Q) {
}

void BhatiaSinghEntropy::Add(double p) {
	m_value += p*sinh(m_q*log(p)/log(2));
	m_count++;
}

double BhatiaSinghEntropy::GetValue(bool normalized) {
	// return - m_value / sinh(m_q);
	return m_value / sinh((-1)*m_q*log((double)m_count));
}

Entropy* BhatiaSinghEntropy::New() {
	return new BhatiaSinghEntropy(m_q);
}
