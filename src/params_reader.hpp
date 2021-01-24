#include <string>
#include <array>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

using ParamsLog = std::vector<std::vector<double>>;

std::vector<std::pair<std::string,ParamsLog>> getParams(std::string jpath);

class Node;
class Link;

class Network
{
public:
	Network(std::string jpath);
	void linkNodes(std::shared_ptr<Node> node_top,
								 std::shared_ptr<Node> node_bot,
								 std::vector<std::vector<double>> params);
	void addBotLink(std::shared_ptr<Node> node,
									std::shared_ptr<Link> &link_candidate);
	void addTopLink(std::shared_ptr<Node> node,
									std::shared_ptr<Link> &link_candidate);
	std::vector<std::pair<std::string,ParamsLog>> getStitchConfig();

private:
	std::map<std::string,std::shared_ptr<Node>> _nodes;
	std::shared_ptr<Node> _node_top;
	std::vector<std::shared_ptr<Link>> _links;
};

class Node
{
friend Network;
friend std::ostream& operator<<(std::ostream &os,const Node &node);

public:
	Node(const std::string ID);

private:
	std::string _ID;
	std::weak_ptr<Link> _link_top;
	std::weak_ptr<Link> _link_bot;
};

class Link
{
friend Network;
friend std::ostream& operator<<(std::ostream &os,const std::weak_ptr<Link> &link_weakptr);

public:
	Link(std::shared_ptr<Node> node_top,
			 std::shared_ptr<Node> node_bot,
			 const std::vector<std::vector<double>> &homo_params,
			 const int link_strength);

private:
	std::shared_ptr<Node> _node_top;
	std::shared_ptr<Node> _node_bot;
	const std::vector<std::vector<double>> _homo_params;
	const int _link_strength;
};

std::ostream& operator<<(std::ostream &os,const Node &node);
std::ostream& operator<<(std::ostream &os,const std::weak_ptr<Link> &link_weakptr);

std::array<std::string,2> splitIDPair(const std::string ID_pair);
