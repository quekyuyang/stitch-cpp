#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <array>
#include <memory>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "params_reader.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<std::pair<std::string,ParamsLog>> getParams(std::string jpath)
{
	Network network(jpath);
	return network.getStitchConfig();
}



Network::Network(std::string jpath)
{
	std::ifstream jfile(jpath);
	json j;
	jfile >> j;

	for (auto &[key,value] : j.items())
	{
		auto ID_pair = splitIDPair(key);

		auto [node1,success1] = _nodes.emplace(ID_pair[0],
			std::make_shared<Node>(ID_pair[0]));
		auto [node2,success2] = _nodes.emplace(ID_pair[1],
			std::make_shared<Node>(ID_pair[1]));

		linkNodes(node1->second,node2->second,value["params"]);
	}
}

void Network::linkNodes(std::shared_ptr<Node> node_top,
							 					std::shared_ptr<Node> node_bot,
							 					std::vector<std::vector<double>> params)
{
	const int link_strength = params.size();
	auto link = std::make_shared<Link>(node_top,node_bot,params,link_strength);

	addBotLink(node_top,link);
	addTopLink(node_bot,link);
}

void Network::addBotLink(std::shared_ptr<Node> node,
												 std::shared_ptr<Link> &link_candidate)
{
	bool candidate_approved = false;
	if (auto link_bot = node->_link_bot.lock())
	{
		if (link_candidate->_link_strength > link_bot->_link_strength)
		{
			candidate_approved = true;
			for (auto it=_links.begin();it!=_links.end();it++)
			{
				if (*it == link_bot)
				{
					_links.erase(it);
					break;
				}
			}
		}
	}
	else
		candidate_approved = true;

	if (candidate_approved)
	{
		_links.push_back(link_candidate);
		node->_link_bot = link_candidate;
	}
}

void Network::addTopLink(std::shared_ptr<Node> node,
												 std::shared_ptr<Link> &link_candidate)
{
	bool candidate_approved = false;
	if (auto link_top = node->_link_top.lock())
	{
		if (link_candidate->_link_strength > link_top->_link_strength)
		{
			candidate_approved = true;
			for (auto it=_links.begin();it!=_links.end();it++)
			{
				if (*it == link_top)
				{
					_links.erase(it);
					break;
				}
			}
		}
	}
	else
		candidate_approved = true;

	if (candidate_approved)
	{
		_links.push_back(link_candidate);
		node->_link_top = link_candidate;
	}
}

std::vector<std::pair<std::string,ParamsLog>> Network::getStitchConfig()
{
	for (const auto &[ID,node] : _nodes)
	{
		// Look for node without a _link_top
		if (!node->_link_top.lock())
			_node_top = node;
	}

	// Create object to return with top node as first entry
	std::vector<std::pair<std::string,ParamsLog>> stitch_config;
	std::vector<double> eye_params{1,0,0,0,1,0,0,0,1};
	ParamsLog eye_paramslog = {eye_params}; //paramslog with only one entry
	stitch_config.push_back(std::make_pair(_node_top->_ID,eye_paramslog));

	std::shared_ptr<Node> node = _node_top;
	std::vector<std::shared_ptr<Node>> explored;
	while (true)
	{
		if (std::find(explored.begin(),explored.end(),node)!=explored.end())
			throw std::runtime_error("Recursive node links");
		explored.push_back(node);

		if (auto link_bot = node->_link_bot.lock())
		{
			node = link_bot->_node_bot;
			stitch_config.push_back(std::make_pair(node->_ID,link_bot->_homo_params));
		}
		else
			break;
	}

	assert(_nodes.size() == explored.size());
	return stitch_config;
}





Node::Node(const std::string ID)
	: _ID(ID)
{}

Link::Link(std::shared_ptr<Node> node_top,
		 			 std::shared_ptr<Node> node_bot,
		 		 	 const std::vector<std::vector<double>> &homo_params,
					 const int link_strength)
	: _node_top(node_top), _node_bot(node_bot),
	  _homo_params(homo_params), _link_strength(link_strength)
{}


std::ostream& operator<<(std::ostream &os,const std::weak_ptr<Link> &link_weakptr)
{
	if (const auto link_sharedptr = link_weakptr.lock())
	{
		os << "node_top: " << *(link_sharedptr->_node_top) << std::endl;
		os << "          node_bot: " << *(link_sharedptr->_node_bot) << std::endl;
	}
	else
		os << std::endl;

	return os;
}

std::ostream& operator<<(std::ostream &os,const Node &node)
{
	os << node._ID;
	return os;
}


std::array<std::string,2> splitIDPair(const std::string ID_pair)
{
	auto pos_dash = ID_pair.find("-");

	std::array<std::string,2> ID_pair_split;
	ID_pair_split[0] = ID_pair.substr(0,pos_dash);
	ID_pair_split[1] = ID_pair.substr(pos_dash+1);

	return ID_pair_split;
}
