#include <string>
#include <array>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

class Node;
class Link;

class Network
{
public:
	Network(std::map<std::string,cv::Mat> imgs,std::string jpath);
	void linkNodes(Node &node_top,Node &node_bot,std::vector<std::vector<double>> params);
	void addBotLink(Node &node,std::shared_ptr<Link> &link_candidate);
	void addTopLink(Node &node,std::shared_ptr<Link> &link_candidate);

private:
	std::vector<std::shared_ptr<Link>> _links;
};

class Node
{
friend Network;
friend std::ostream& operator<<(std::ostream &os,const Node &node);

public:
	Node(cv::Mat &img,const std::string ID);

private:
	cv::Mat _img;
	std::string _ID;
	std::weak_ptr<Link> _link_top;
	std::weak_ptr<Link> _link_bot;
};

class Link
{
friend Network;
friend std::ostream& operator<<(std::ostream &os,const std::weak_ptr<Link> &link_weakptr);

public:
	Link(Node &node_top,Node &node_bot,const cv::Mat homo_mat,const int link_strength);

private:
	Node &_node_top;
	Node &_node_bot;
	const cv::Mat _homo_mat;
	const int _link_strength;
};

std::ostream& operator<<(std::ostream &os,const Node &node);
std::ostream& operator<<(std::ostream &os,const std::weak_ptr<Link> &link_weakptr);

std::array<std::string,2> splitIDPair(const std::string ID_pair);