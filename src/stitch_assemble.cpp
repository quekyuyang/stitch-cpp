#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <array>
#include <opencv2/opencv.hpp>
#include "stitch_assemble.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Network::Network(std::map<std::string,cv::Mat> imgs,std::string jpath)
{
	std::ifstream jfile(jpath);
	json j;
	jfile >> j;

	std::map<std::string,Node> nodes;

	for (auto &[key,value] : j.items())
	{
		auto ID_pair = splitIDPair(key);

		auto [node1,success1] = nodes.emplace(ID_pair[0],Node(imgs[ID_pair[0]],ID_pair[0]));
		auto [node2,success2] = nodes.emplace(ID_pair[1],Node(imgs[ID_pair[1]],ID_pair[1]));

		linkNodes(node1->second,node2->second,value["params"]);
	}

	for (auto &[ID,node] : nodes)
	{
		std::cout << node << std::endl;
		std::cout << "link_top: " << node._link_top;
		std::cout << "link_bot: " << node._link_bot;
	}
}

void Network::linkNodes(Node &node_top,Node &node_bot,std::vector<std::vector<double>> params)
{
	const int link_strength = params.size();

	cv::Mat params_mat;
	for (auto param : params)
	{
		params_mat.push_back(cv::Mat(param).reshape(1,1));
	}
	cv::Mat params_mean;
	cv::reduce(params_mat,params_mean,0,cv::REDUCE_AVG);
	
	const cv::Mat homo_mat = params_mean.reshape(1,3);
	auto link = std::make_shared<Link>(node_top,node_bot,homo_mat,link_strength);

	addBotLink(node_top,link);
	addTopLink(node_bot,link);
}
/*
	cv::Mat stitch;
	cv::warpPerspective(node_bot._img,stitch,homo_mat,cv::Size(1844,1700));

	cv::Mat ROI = stitch.rowRange(0,node_top._img.rows);
	node_top._img.copyTo(ROI);
}*/

void Network::addBotLink(Node &node,std::shared_ptr<Link> &link_candidate)
{
	bool candidate_approved = false;
	if (auto link_bot = node._link_bot.lock())
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
		node._link_bot = link_candidate;
	}
}

void Network::addTopLink(Node &node,std::shared_ptr<Link> &link_candidate)
{
	bool candidate_approved = false;
	if (auto link_top = node._link_top.lock())
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
		node._link_top = link_candidate;
	}
}


Node::Node(cv::Mat &img,const std::string ID)
	: _img(img),_ID(ID)
{}

Link::Link(Node &node_top,Node &node_bot,const cv::Mat homo_mat,const int link_strength)
	: _node_top(node_top), _node_bot(node_bot), _homo_mat(homo_mat), _link_strength(link_strength)
{}


std::ostream& operator<<(std::ostream &os,const std::weak_ptr<Link> &link_weakptr)
{
	if (const auto link_sharedptr = link_weakptr.lock())
	{
		os << "node_top: " << link_sharedptr->_node_top << std::endl;
		os << "          node_bot: " << link_sharedptr->_node_bot << std::endl;
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