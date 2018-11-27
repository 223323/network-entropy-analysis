#ifndef ENTROPY_H
#define ENTROPY_H

class Entropy {
	public:
		Entropy();
		virtual void Add(double p);
		void SetCount(int count);
		virtual double GetValue(bool normalized=true);
	protected:
		double m_value;
		int m_count;
};

class TsalisEntropy : public Entropy {
	public:
		TsalisEntropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
	private:
		double m_q;
};

#endif
