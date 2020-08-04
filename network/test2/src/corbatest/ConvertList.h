#ifndef STI_NETWORK_CONVERTLIST_H
#define STI_NETWORK_CONVERTLIST_H

#include <string>
#include <set>
#include <vector>
#include <map>

namespace STI
{
namespace Network
{


template<typename In, typename Out, template<class> typename Seq>
struct ConvertList
{
	bool operator()(const std::vector<In>& input, Seq<Out>& output)
	{
		output.length(static_cast<unsigned>(input.size()));

		std::vector<In>::const_iterator in = input.begin();
		for (unsigned i = 0; i < output.length() && in != input.end(); ++i, ++in) {
			convert<In, Out>(*in, output[i]);
		}
		return (input.size() == output.length());
	}

	bool operator()(const std::set<In>& input, Seq<Out>& output)
	{
		output.length(static_cast<unsigned>(input.size()));

		std::set<In>::const_iterator in = input.begin();
		for (unsigned i = 0; i < output.length() && in != input.end(); ++i, ++in) {
			convert<In, Out>(*in, (Out&)output[i]);
		}
		return (input.size() == output.length());
	}

	bool operator()(const Seq<In>& input, std::vector<Out>& output)
	{
		output.clear();
		for (unsigned i = 0; i < input.length(); ++i) {
			output.push_back(convert<In, Out>(input[i]));
		}
		return (output.size() == input.length());
	}

	template<typename Any>
	bool operator()(const std::map<Any, In>& input, Seq<Out>& output)
	{
		output.length(input.size());

		std::map<Any, In>::const_iterator in = input.begin();
		for (unsigned i = 0; i < output.length() && in != input.end(); ++i, ++in) {
			//output[i] = convert<In, Out>(in->second);
			convert<In, Out>(in->second, (Out&)output[i]);
		}
		return (input.size() == output.length());
	}
};


} //Network
} //STI

#endif

