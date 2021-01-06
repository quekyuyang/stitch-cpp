#include <vector>
#include "stitch_compute.hpp"

class ParamsLog
{
public:
  void addHomoParams(std::vector<double> homo_params);
  void addNInliers(int n_inliers);
  std::vector<std::vector<double>> getHomoParams() const;
  std::vector<int> getNInliers() const;

private:
  std::vector<std::vector<double>> _homo_params_log;
  std::vector<int> _n_inliers_log;
};

class ParamsLogManager
{
public:
  void addNodeData(std::vector<Node> &nodes);
  void saveJson(std::string filepath);

private:
  std::map<std::string,ParamsLog> _params_logs;
};