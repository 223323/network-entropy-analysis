#include "Entropy.h"
#include <cmath>


// Entropy
// ----------------------------------------
Entropy::Entropy(double q) : m_value(0.0), m_count(0), m_q(q) {}
void Entropy::Add(double p) {
	
}

void Entropy::SetCount(int count) {
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
	if(p == 0) {
		m_value = 0;
	} else {
		m_value += p * (-log2(p));
	}
	m_count++;
}

double ShannonEntropy::GetValue(bool normalized) {
	if(normalized) {
		if(m_count <= 1) {
			return m_value;
		} else {
			return m_value * 100.0 / log2((double)m_count);
		}
	} else {
		return m_value;
	}
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
		return std::abs(1 - m_value) * (denom > 0 ? (100.0 / denom) : 0);
	} else {
		return m_value;
	}
}

Entropy* TsalisEntropy::New() {
	return new TsalisEntropy(m_q);
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
	if(normalized) {
		if (m_value == 0) return 0;
		double denom = log(pow(m_count, (1 - m_q)));
		return std::abs( log(m_value) * (denom > 0 ? (100.0 / denom) : 100.0) );
	} else {
		return m_value;
	}
}
Entropy* RenyiEntropy::New() {
	return new RenyiEntropy(m_q);
}
