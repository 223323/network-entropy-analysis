#include "Entropy.h"
#include <cmath>

Entropy::Entropy() : m_value(0.0), m_count(0) {}

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

TsalisEntropy::TsalisEntropy(double q) {
	m_q = q;
}

void TsalisEntropy::Add(double p) {
	m_value += pow(p, m_q);
}
		
double TsalisEntropy::GetValue(bool normalized) {
	if(normalized) {
		return (1-m_value) * 100.0;
	} else {
		return m_value;
	}
}
