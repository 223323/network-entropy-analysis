#include "Entropy.h"
#include <cmath>

Entropy::Entropy() : m_value(0.0), m_count(0) {}

// Shannon
// ----------------------------------------
void Entropy::Add(double p) {
	if(p == 0) {
		m_value = 0;
	} else {
		m_value += p * (-log2(p));
	}
	m_count++;
}

void Entropy::SetCount(int count) {
	m_count = count;
}

double Entropy::GetValue(bool normalized) {
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

// Tsalis
// ----------------------------------------
TsalisEntropy::TsalisEntropy(double q) {
	m_q = q;
	m_count=0;
	m_value=0;
}

void TsalisEntropy::Add(double p) {
	m_value += pow(p, m_q);
	m_count++;
}
		
double TsalisEntropy::GetValue(bool normalized) {
	if(normalized) {
		double denom = 1 - pow(m_count, 1-m_q);
		return (1-m_value) * (denom > 0 ? (100.0 / denom) : 0);
	} else {
		return m_value;
	}
	
}


// Renyi entropy
// ----------------------------------------
RenyiEntropy::RenyiEntropy(double Q) {
	m_q = Q;
	m_value=0;
	m_count=0;
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
