#ifndef ENTROPY_H
#define ENTROPY_H
#include <stdint.h>
class Entropy {
	public:
		Entropy(double q);
		virtual void Add(double p);
		void SetCount(uint32_t count);
		void SetQ(double q);
		virtual double GetValue(bool normalized=true);
		virtual Entropy* New();
	protected:
		double m_value;
		uint32_t m_count;
		double m_q;
};

class ShannonEntropy : public Entropy {
	public:
		ShannonEntropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};

class TsalisEntropy : public Entropy {
	public:
		TsalisEntropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};

class Tsalis2Entropy : public Entropy {
	public:
		Tsalis2Entropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};

class RenyiEntropy : public Entropy {
	public:
		RenyiEntropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};
/*
class Renyi2Entropy : public Entropy {
	public:
		Renyi2Entropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};
*/
class UbriacoEntropy : public Entropy {
	public:
		UbriacoEntropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};

class BhatiaSinghEntropy : public Entropy {
	public:
		BhatiaSinghEntropy(double q);
		void Add(double p);
		double GetValue(bool normalized=true);
		virtual Entropy* New();
	private:
};

#endif
