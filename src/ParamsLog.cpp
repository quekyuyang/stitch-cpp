#include <vector>
#include <fstream>
#include "ParamsLog.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ParamsLogManager::addNodeData(std::vector<Node> &nodes)
{
  for (const auto &node : nodes)
  {
    std::string ID_pair(node.getID() + "-" + node.getBestLink().getTargetID());

    const cv::Mat &homo_mat = node.getBestLink().getHomoMat();
    if (homo_mat.empty())
      continue;

    std::vector<double> homo_params;
    if (homo_mat.isContinuous())
      homo_params.assign(homo_mat.begin<double>(),homo_mat.end<double>());
    else
      throw std::runtime_error("Homography matrix not continuous");

    _params_logs[ID_pair].addHomoParams(homo_params);
    _params_logs[ID_pair].addNInliers(node.getBestLink().getNInliers());
  }
}

void ParamsLogManager::saveJson(std::string filepath)
{
  json jdata;
  for (const auto &entry : _params_logs)
  {
    jdata[entry.first]["params"] = entry.second.getHomoParams();
    jdata[entry.first]["n_inliers"] = entry.second.getNInliers();
  }

  std::ofstream file(filepath);
  file << jdata;
}

void ParamsLog::addHomoParams(std::vector<double> homo_params)
{
  _homo_params_log.push_back(homo_params);
}

void ParamsLog::addNInliers(int n_inliers)
{
  _n_inliers_log.push_back(n_inliers);
}

std::vector<std::vector<double>> ParamsLog::getHomoParams() const {return _homo_params_log;}
std::vector<int> ParamsLog::getNInliers() const {return _n_inliers_log;}