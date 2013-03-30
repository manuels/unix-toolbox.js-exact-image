#include "ContourUtility.hh"

class LogoRepresentation
{
public:

  LogoRepresentation(Contours* logo_contours,
		     unsigned int max_feature_no,
		     unsigned int max_avg_tolerance,
		     unsigned int reduction_shift,
		     double maximum_angle,
		     double angle_step);


  ~LogoRepresentation();

  double Score(Contours* image);

  // updated after call to score
  std::pair<int, int> logo_translation;
  double rot_angle;
  std::vector< std::pair <Contours::Contour*, Contours::Contour*> > mapping; // logo contour, image contour

  // calculates the (unrotatated) logo translation after -rot_angle image rotation around (rx,ry)
  const std::pair<int, int>& CalculateInverseTranslation(int rx, int ry);
  std::pair<int, int> inverse_translation;

protected:
  friend class MatchSorter;

  double N_M_Match(unsigned int set, unsigned int& pivot);
  double PrecisionScore();

  void RotatedCentroidPosition(double& rx, double& ry);
  bool OptimizeAngle(double& score, double delta);
  bool OptimizeHTranslation(double& score, int delta);
  bool OptimizeVTranslation(double& score, int delta);
  bool Optimize(double& score);

  Contours* source;
  unsigned int tolerance;
  unsigned int shift;
  double rot_max;
  double rot_step;
  double centerx;
  double centery;
  unsigned int logo_set_count;
  unsigned int total_contour_length;

  class Match;

  struct LogoContourData
  {
    Contours::Contour* contour;
    double rx;
    double ry;
    std::vector <Match*> matches;
    unsigned int n_to_n_match_index;
  };

  struct ImageContourData
  {
    Contours::Contour* contour;
    double rx;
    double ry;
  };

  class Match
  {
  public:
    unsigned int length;
    double score;
    double transx;
    double transy;
    Contours::Contour* cimg;

    Match(const ImageContourData& image,
	  const LogoContourData& logo,
	  int tolerance,
	  int shift,
	  unsigned int original_logo_length,
	  Contours::Contour* icimg);

    double TransScore(double tx, double ty);
  };



  std::vector < std::vector <LogoContourData> > logo_sets;
  std::vector < unsigned int > logo_set_map;

  std::vector < ImageContourData > image_set;
};


