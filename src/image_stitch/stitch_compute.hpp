#ifndef STITCH_COMPUTE_HPP
#define STITCH_COMPUTE_HPP

#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include "image_struct.hpp"

class Node;
class Network;
class StitchComputer;

class Link
{
friend Network;
friend Node;

public:
  Link(const std::string target_ID="",const int n_inliers=0,const cv::Mat homo_mat=cv::Mat());
  std::string getTargetID() const;
  int getNInliers() const;
  cv::Mat getHomoMat() const;

private:
  std::string _target_ID;
  int _n_inliers;
  cv::Mat _homo_mat;
};




class Node
{
friend Network;

public:
  Node (std::string ID="");
  void addLink(const std::string target_ID,const int n_inliers,const cv::Mat homo_mat);
  void findBestLink();
  std::string getID() const;
  Link getBestLink() const;

private:
  const std::string _ID;
  std::map<std::string,Link> _links;
  Link _best_link;
};




class Network
{
friend StitchComputer;

public:
  void addNode(std::string ID);
  void addLink(const std::string ID1,const std::string ID2,const int n_inliers,const cv::Mat);
  void findBestLinks();
  std::vector<Node> getNodes() const;

private:
  std::map<std::string,Node> _nodes;
};




class StitchComputer
{
public:
  StitchComputer(std::map<std::string,cv::Mat> imgs,const int nfeatures,
                 const int max_error_inlier,const int min_n_inliers);
  std::vector<cv::Rect> getBotHalfROI(const std::vector<cv::Rect> &ROIs,const std::string &ID);
  std::vector<cv::Rect> getTopHalfROI(const std::vector<cv::Rect> &ROIs,const std::string &ID);
  void autoLink(std::vector<std::string> IDs,const std::vector<cv::Rect> ROIs_features);
  void manualLink(std::string ID1,std::string ID2,
    std::vector<cv::Rect> ROIs_features1,std::vector<cv::Rect>ROIs_features2,
    const bool histequal);
  std::vector<Node> getNodes();

private:
  const int _max_error_inlier;
  const int _min_n_inliers;
  const int _nfeatures;
  std::map<std::string,Image> _images;
  Network _network;
};




void getMatches(cv::Mat des1,cv::Mat des2,std::vector<cv::DMatch> &matches);


#endif
