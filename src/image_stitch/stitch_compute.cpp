#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "stitch_compute.hpp"

StitchComputer::StitchComputer(std::map<std::string,cv::Mat> imgs,const int nfeatures,
                               const int max_error_inlier,const int min_n_inliers)
  : _max_error_inlier(max_error_inlier), _min_n_inliers(min_n_inliers),
    _network(Network()), _nfeatures(nfeatures)
{
  for (auto &[ID,img] : imgs)
  {
    _images.emplace(ID,Image(img,nfeatures,ID));
    _network.addNode(ID);
  }
}

std::vector<cv::Rect> StitchComputer::getBotHalfROI(const std::vector<cv::Rect> &ROIs,const std::string &ID)
{
  std::vector<cv::Rect> ROIs_bot_half;
  for (const auto &ROI : ROIs)
  {
    const cv::Point2i tl_new(ROI.tl().x,std::max(ROI.tl().y,_images[ID].getImg().rows/2));
    ROIs_bot_half.push_back(cv::Rect(tl_new,ROI.br()));
  }
  return ROIs_bot_half;
}

std::vector<cv::Rect> StitchComputer::getTopHalfROI(const std::vector<cv::Rect> &ROIs,const std::string &ID)
{
  std::vector<cv::Rect> ROIs_top_half;
  for (const auto &ROI : ROIs)
  {
    const cv::Point2i br_new(ROI.br().x,std::min(ROI.br().y,_images[ID].getImg().rows/2));
    ROIs_top_half.push_back(cv::Rect(ROI.tl(),br_new));
  }
  return ROIs_top_half;
}

void StitchComputer::manualLink(std::string ID1,std::string ID2,
                                std::vector<cv::Rect> ROIs_features1,
                                std::vector<cv::Rect> ROIs_features2,
                                const bool histequal)
{
  std::vector<cv::KeyPoint> kps1,kps2;
  cv::Mat des1,des2;
  _images[ID1].getKpsAndDes(_nfeatures,histequal,kps1,des1,ROIs_features1);
  _images[ID2].getKpsAndDes(_nfeatures,histequal,kps2,des2,ROIs_features2);

  std::vector<cv::DMatch> matches;
  getMatches(des1,des2,matches);
  std::vector<cv::Point2f> pts1_match,pts2_match;
  for (const auto &match : matches)
  {
    pts1_match.push_back(kps1[match.queryIdx].pt);
    pts2_match.push_back(kps2[match.trainIdx].pt);
  }

  cv::Mat mask;
  const cv::Mat homo_mat = cv::findHomography(pts2_match,pts1_match,cv::RANSAC,
                                              _max_error_inlier,mask);
  const int n_inliers = cv::sum(mask)[0];

  _network.addLink(ID1,ID2,n_inliers,homo_mat);
}

void StitchComputer::autoLink(std::vector<std::string> IDs,const std::vector<cv::Rect> ROIs_features)
{
  for (const auto ID1 : IDs)
  {
    for (const auto ID2 : IDs)
    {
      if (ID1 == ID2)
        continue;

      auto ROIs_image1 = getBotHalfROI(ROIs_features,ID1);
      auto ROIs_image2 = getTopHalfROI(ROIs_features,ID2);

      std::vector<cv::KeyPoint> kps1,kps2;
      cv::Mat des1,des2;
      _images[ID1].getKpsAndDes(_nfeatures,true,kps1,des1,ROIs_image1);
      _images[ID2].getKpsAndDes(_nfeatures,true,kps2,des2,ROIs_image2);

      std::vector<cv::DMatch> matches;
      getMatches(des1,des2,matches);

      std::vector<cv::Point2f> pts1_match,pts2_match;
      for (const auto &match : matches)
      {
        pts1_match.push_back(kps1[match.queryIdx].pt);
        pts2_match.push_back(kps2[match.trainIdx].pt);
      }

      cv::Mat mask;
      const cv::Mat homo_mat = cv::findHomography(pts2_match,pts1_match,cv::RANSAC,_max_error_inlier,mask);
      const int n_inliers = cv::sum(mask)[0];

      if (n_inliers > _min_n_inliers)
        _network.addLink(ID1,ID2,n_inliers,homo_mat);
    }
  }
}

std::vector<Node> StitchComputer::getNodes()
{
  _network.findBestLinks();
  return _network.getNodes();
}




void Network::addNode(std::string ID)
{
  _nodes.emplace(ID,Node(ID));
}

void Network::addLink(const std::string ID1,const std::string ID2,const int n_inliers,const cv::Mat homo_mat)
{
  _nodes[ID1].addLink(ID2,n_inliers,homo_mat);
}

void Network::findBestLinks()
{
  for (auto &[ID,node] : _nodes)
    node.findBestLink();
}

std::vector<Node> Network::getNodes() const
{
  std::vector<Node> nodes;
  for (auto &[ID,node] : _nodes)
  {
    nodes.push_back(node);
    std::cout << node._ID << std::endl;
    std::cout << node.getBestLink().getTargetID() << std::endl;
  }


  return nodes;
}




Node::Node(std::string ID)
  : _ID(ID)
{}

void Node::addLink(const std::string target_ID,const int n_inliers,const cv::Mat homo_mat)
{
  if (_links.count(target_ID) == 0)
    _links.emplace(target_ID,Link(target_ID,n_inliers,homo_mat));
  else
    throw std::runtime_error("Attempt to add already existing link");
}

void Node::findBestLink()
{
  int highest_n_inliers = 0;
  std::string ID_most_inliers;
  for (auto &[ID,link] : _links)
  {
    if (link._n_inliers > highest_n_inliers)
    {
      highest_n_inliers = link._n_inliers;
      ID_most_inliers = ID;
    }
  }
  _best_link = Link(_links[ID_most_inliers]);
}

std::string Node::getID() const {return _ID;}
Link Node::getBestLink() const {return _best_link;}



Link::Link(const std::string target_ID,const int n_inliers,const cv::Mat homo_mat)
  : _target_ID(target_ID), _n_inliers(n_inliers), _homo_mat(homo_mat)
{}

std::string Link::getTargetID() const {return _target_ID;}
int Link::getNInliers() const {return _n_inliers;}
cv::Mat Link::getHomoMat() const {return _homo_mat;}




void getMatches(cv::Mat des1,cv::Mat des2,std::vector<cv::DMatch> &matches)
{
  cv::BFMatcher bf(cv::NORM_HAMMING,true);
  bf.match(des1,des2,matches);
  std::sort(matches.begin(),matches.end());
}
