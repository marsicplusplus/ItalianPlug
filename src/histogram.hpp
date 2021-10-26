#ifndef __HISTOGRAM_HPP__
#define __HISTOGRAM_HPP__

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>

class Histogram{
	public:
		Histogram(int b) :  m_bins{b}{};

		inline std::string toString(){
			std::ostringstream s;
			for(const auto &a : histogram){
				s << a.second << ":";
			}
			std::string ret = s.str();
			ret.erase(ret.length()-1);
			return ret;
		}

		inline void normalize(){
			float sum = 0;
			for(auto &a : histogram){
				sum+=a.second;
			}
			for(auto &a : histogram){
				a.second /= sum;
			}
		}

	inline void populate(std::vector<float> &values){
		float range = values[values.size() - 1];
		m_width = range/(float) m_bins;
		float bin = 0.0f;
		for(int i = 0; i < m_bins; ++i){
			histogram[bin] = 0;
			bin += m_width;
		}
		bin = 0.0f;
		for(auto a : values){
			if(a > bin + m_width) {
				if(histogram.find(bin+m_width) != histogram.end())
					bin += m_width;
			}
			++histogram[bin];
		}
	}

	private:
		float m_width;
		int m_bins;
		std::map<float, float> histogram;
};

#endif
