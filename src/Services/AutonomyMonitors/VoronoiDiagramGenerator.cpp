/*
 * The author of this software is Steven Fortune.  Copyright (c) 1994 by AT&T
 * Bell Laboratories.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * Author: Guohui and Sriram
 */

#include "VoronoiDiagramGenerator.h"

VoronoiDiagramGenerator::VoronoiDiagramGenerator() {
  siteIndex = 0;
  allEdges = 0;
  iteratorEdges = 0;
  gapThreshold = 0;
  this->searchAreaType = 1;
}

VoronoiDiagramGenerator::~VoronoiDiagramGenerator() {}

void VoronoiDiagramGenerator::hashTableOfSiteVDvertices() {
   for(int i = 0; i < VDedgeSetHelper.size(); i++) {
    VDvertices[VDedgeSetHelper[i].first].push_back(VDedgeSet[i].first);
    VDvertices[VDedgeSetHelper[i].first].push_back(VDedgeSet[i].second);
    VDvertices[VDedgeSetHelper[i].second].push_back(VDedgeSet[i].first);
    VDvertices[VDedgeSetHelper[i].second].push_back(VDedgeSet[i].second);
  }
  std::unordered_map<int, std::vector<struct Point> > vdupdate = VDvertices;
  for(int i = 0; i < sites.size(); i++) {
    VDvertices[sites[i].sitemarker] = vdupdate[sites[i].sitemarker];
    sort(VDvertices[sites[i].sitemarker].begin(), VDvertices[sites[i].sitemarker].end(), [](const Point &a, const Point &b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
      });
    for(int j = 0; j < VDvertices[sites[i].sitemarker].size() - 1; j++) {
      double diff = VDvertices[sites[i].sitemarker][j].y - VDvertices[sites[i].sitemarker][j+1].y;
      if(diff > -EPS && diff < EPS) VDvertices[sites[i].sitemarker].erase(VDvertices[sites[i].sitemarker].begin() + j+1);
    }
  }
  
}

bool VoronoiDiagramGenerator::generateVDrectangle(std::vector<std::pair<double, double>> viewAngleList, double lowerBorderX, double upperBorderX, double lowerBorderY, double upperBorderY, double minDist) {
  if(viewAngleList.empty()) return false;
  
  gapThreshold = minDist;
  siteNumber = viewAngleList.size();
  //triangulate = 0;
  xmin = viewAngleList[0].first;
  ymin = viewAngleList[0].second;
  xmax = viewAngleList[0].first;
  ymax = viewAngleList[0].second;
  
  sites.resize(siteNumber);
  for(int i = 0; i < siteNumber; i++) {
    sites[i].coord.x = viewAngleList[i].first;
    sites[i].coord.y = viewAngleList[i].second;
    sites[i].sitemarker = i;
    sites[i].visited = 0;

    if(viewAngleList[i].first < xmin) xmin = viewAngleList[i].first;
    else if(viewAngleList[i].first > xmax) xmax = viewAngleList[i].first;

    if(viewAngleList[i].second < ymin) ymin = viewAngleList[i].second;
    else if(viewAngleList[i].second > ymax) ymax = viewAngleList[i].second;
  }
  
  sort(sites.begin(), sites.end(), sort_pred());
  siteIndex = 0;
  VDverticesNumber = 0;
  edgeNumber = 0;
  if(lowerBorderX > upperBorderX) std::swap(lowerBorderX, upperBorderX);
  if(lowerBorderY > upperBorderY) std::swap(lowerBorderY, upperBorderY);
  
  borderMinX = lowerBorderX;
  borderMinY = lowerBorderY;
  borderMaxX = upperBorderX;
  borderMaxY = upperBorderY;
  
  siteIndex = 0;
  voronoiOperation();
 
  return true;
}

bool VoronoiDiagramGenerator::generateVDcircle(std::vector<std::pair<double, double> > viewAngleList, std::pair<double, double> Center, double Radius,  double minDist) {
  if(viewAngleList.empty()) return false;
  
  double upperBorderX = Center.first + Radius;
  double lowerBorderX = Center.first - Radius;
  double upperBorderY = Center.second + Radius;
  double lowerBorderY = Center.second - Radius;

  generateVDrectangle(viewAngleList, lowerBorderX, upperBorderX, lowerBorderY, upperBorderY, minDist);
  
  struct Point stCenter;
  stCenter.x = Center.first;
  stCenter.y = Center.second;
  std::cout << VDedgeSet.size() << std::endl;
  for(int i = 0; i < VDedgeSet.size(); i++) {
    struct Point *s1 = &VDedgeSet[i].first;
    struct Point *s2 = &VDedgeSet[i].second;
    if(s1 == (struct Point *) NULL || s2 == (struct Point *) NULL) continue;
    double A = distPoint(*s1, *s2)*distPoint(*s1, *s2);
    double B = 2*((s2->x - s1->x)*(s1->x - Center.first) + (s2->y - s1->y)*(s1->y - Center.second));
    double C = Center.first * Center.first + Center.second * Center.second + s1->x * s1->x + s1->y * s1->y - 2*(Center.first * s1->x + Center.second * s1->y) - Radius*Radius;
    double delta = B*B - 4*A*C;
    struct Point newp;
    if(distPoint(*s1, stCenter) <= Radius && distPoint(*s2, stCenter) <= Radius) {
      std::cout << "-------in---------" << std::endl;
    } else if(distPoint(*s1, stCenter) > Radius && distPoint(*s2, stCenter) > Radius) {
      std::cout << "-----------------" << std::endl;
    } else {
      if(delta > EPS) {
        std::cout << "-------out---------" << std::endl;
        double u1 = (-B + sqrt(delta))/(2*A);
        double u2 = (-B - sqrt(delta))/(2*A);
        if((u1 >= EPS && u1 <= 1) && (u2 < -EPS || u2 > 1)) {
          if(distPoint(*s1, stCenter) <= Radius) {
            s2->x = s1->x + u1*(s2->x - s1->x);
            s2->y = s1->y + u1*(s2->y - s1->y);
          }
          else{
            s1->x = s1->x + u1*(s2->x - s1->x);
            s1->y = s1->y + u1*(s2->y - s1->y);
          }
        } else if((u2 >= EPS && u2 <= 1) && (u1 < -EPS || u1 > 1)) {
          if(distPoint(*s1, stCenter) <= Radius) {
            s2->x = s1->x + u2*(s2->x - s1->x);
            s2->y = s1->y + u2*(s2->y - s1->y);
          }
          else{
            s1->x = s1->x + u2*(s2->x - s1->x);
            s1->y = s1->y + u2*(s2->y - s1->y);
          }
        } else if((u2 >= EPS && u2 <= 1) && (u1 >= EPS || u1 <= 1)) {
          s1->x = s1->x + u1*(s2->x - s1->x);
          s1->y = s1->y + u1*(s2->y - s1->y);
          s2->x = s1->x + u2*(s2->x - s1->x);
          s2->y = s1->y + u2*(s2->y - s1->y);
        }
      }
    }
  }
  
  return true;
}

double VoronoiDiagramGenerator::distPoint(struct Point s1, struct Point s2) {
  return sqrt(pow(s1.x - s2.x, 2) + pow(s1.y - s2.y, 2));
}


bool VoronoiDiagramGenerator::generateVDpolygon(std::vector<std::pair<double, double> > viewAngleList, std::vector<std::pair<double, double> > polyBoundary, double minDist) {
  if(viewAngleList.empty()) return false;
  
  std::pair<double, double> Xpoint=polyBoundary[0], Ypoint=polyBoundary[0];
  // Find the external rectangle of polyboundary
  for(auto item: polyBoundary) {
    Xpoint.first = std::min(Xpoint.first, item.first); // X-min
    Xpoint.second = std::max(Xpoint.second, item.first); // X-max
    Ypoint.first = std::min(Ypoint.first, item.second); // Y-min
    Ypoint.second = std::max(Ypoint.second, item.second); // Y-max
  }
  std::cout << Xpoint.first << " " << Xpoint.second << Ypoint.first << " " << Ypoint.second << std::endl;
  generateVDrectangle(viewAngleList, Xpoint.first, Xpoint.second, Ypoint.first, Ypoint.second, minDist);

  for(int i = 0; i < VDedgeSet.size(); i++) {
    if(VDedgeSet[i].first.x == Xpoint.first || VDedgeSet[i].first.x == Xpoint.second) {
      for(int j = 0; j < polyBoundary.size()-1; j++) {
        struct Point intersect;
        std::pair<struct Point, struct Point> boundary;
        boundary.first.x = polyBoundary[j].first;
        boundary.first.y = polyBoundary[j].second;
        boundary.second.x = polyBoundary[j+1].first;
        boundary.second.y = polyBoundary[j+1].second;
        
        if(segIntersectSeg(VDedgeSet[i], boundary, intersect)) {
          VDedgeSet[i].first = intersect;
          //break;
        }
      }
    }
    if(VDedgeSet[i].first.y == Ypoint.first || VDedgeSet[i].first.y == Ypoint.second) {
      for(int j = 0; j < polyBoundary.size()-1; j++) {
        struct Point intersect;
        std::pair<struct Point, struct Point> boundary;
        boundary.first.x = polyBoundary[j].first;
        boundary.first.y = polyBoundary[j].second;
        boundary.second.x = polyBoundary[j+1].first;
        boundary.second.y = polyBoundary[j+1].second;
        
        if(segIntersectSeg(VDedgeSet[i], boundary, intersect)) {
          VDedgeSet[i].first = intersect;
          //break;
        }
      }
    }

    if(VDedgeSet[i].second.x == Xpoint.first || VDedgeSet[i].second.x == Xpoint.second) {
      for(int j = 0; j < polyBoundary.size()-1; j++) {
        struct Point intersect;
        std::pair<struct Point, struct Point> boundary;
        boundary.first.x = polyBoundary[j].first;
        boundary.first.y = polyBoundary[j].second;
        boundary.second.x = polyBoundary[j+1].first;
        boundary.second.y = polyBoundary[j+1].second;
        
        if(segIntersectSeg(VDedgeSet[i], boundary, intersect)) {
          VDedgeSet[i].second = intersect;
          //break;
        }
      }
    }
    if(VDedgeSet[i].second.y == Ypoint.first || VDedgeSet[i].second.y == Ypoint.second) {
      for(int j = 0; j < polyBoundary.size()-1; j++) {
        struct Point intersect;
        std::pair<struct Point, struct Point> boundary;
        boundary.first.x = polyBoundary[j].first;
        boundary.first.y = polyBoundary[j].second;
        boundary.second.x = polyBoundary[j+1].first;
        boundary.second.y = polyBoundary[j+1].second;
        
        if(segIntersectSeg(VDedgeSet[i], boundary, intersect)) {
          VDedgeSet[i].second = intersect;
          //break;
        }
      }
    }
  }

  return true;
}

bool VoronoiDiagramGenerator::segIntersectSeg(std::pair<struct Point, struct Point> &edge, std::pair<struct Point, struct Point> boundary, struct Point &intersect) {
  // Check whether two line segments are intersected or not
  //if(edge.first.x > edge.second.x) std::swap(edge.first, edge.second);
  //if(boundary.first.x > boundary.second.x) std::swap(boundary.first, boundary.second);
  //if(std::max(edge.first.x, boundary.first.x) > std::min(edge.second.x, boundary.second.x)) return false;

  // find the intersection
  double x1 = edge.first.x, x2 = edge.second.x;
  double y1 = edge.first.y, y2 = edge.second.y;
  double x3 = boundary.first.x, x4 = boundary.second.x;
  double y3 = boundary.first.y, y4 = boundary.second.y;
  double denominator = (y4 - y3)*(x2 - x1) - (x4 - x3) * (y2 - y1);
  if(denominator > -EPS && denominator < EPS) return false;

  double ua = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
  ua = ua / denominator;
  double ub = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3);
  ub = ub / denominator;

  if((ua <= 1 && ua >= EPS) && (ub <= 1 && ub >= EPS)) {
    intersect.x = x1 + ua * (x2 - x1);
    intersect.y = y1 + ua * (y2 - y1);
  } else return false;
  
  return true;
}

/******************************************************************************
 * Fundamental operations
 */
struct VoronoiEdge* VoronoiDiagramGenerator::HEcreate(struct Edge *edge, int pm) {
  struct VoronoiEdge *answer;
  answer = new struct VoronoiEdge();
  answer->innerVedge = edge;
  answer->ELpm = pm;
  answer->PQnext = (struct VoronoiEdge *) NULL;
  answer->vertex = (struct Site *) NULL;
  //answer->ELvisited = 0;
  return(answer);
}

// Return the next site
struct Site *VoronoiDiagramGenerator::nextone() {
  struct Site *index;
  if(siteIndex < siteNumber) {
    index = &sites[siteIndex];
    siteIndex += 1;
    return index;
  }
  else return (struct Site *) NULL;
}

void VoronoiDiagramGenerator::pushGraphEdge(double x1, double y1, double x2, double y2) {
  GraphEdge* newEdge = new GraphEdge;
  newEdge->next = allEdges;
  allEdges = newEdge;
  newEdge->x1 = x1;
  newEdge->y1 = y1;
  newEdge->x2 = x2;
  newEdge->y2 = y2;
}

/******************************************************************************
 * Edge List for all the edges on the plane
 */
bool VoronoiDiagramGenerator::EdgeListInit() {
  //ELhashsize = 2 * sqrt_siteNumber;
  if(siteNumber == 0) return false;

  //ELhash.resize(ELhashsize);
  EdgeListLeftBorder = HEcreate( (struct Edge *)NULL, 0);
  EdgeListRightBorder = HEcreate( (struct Edge *)NULL, 0);
  
  EdgeListLeftBorder->leftVedge = (struct VoronoiEdge *) NULL;
  EdgeListLeftBorder->rightVedge = EdgeListRightBorder;
  EdgeListRightBorder->leftVedge = EdgeListLeftBorder;
  EdgeListRightBorder->rightVedge = (struct VoronoiEdge *)NULL;

  return true;
}

// Insert newHalfEdge on the right the currentEdge
void VoronoiDiagramGenerator::EdgeListInsert(struct VoronoiEdge *current, struct VoronoiEdge *newVDedge) {
  newVDedge->leftVedge = current;
  newVDedge->rightVedge = current->rightVedge;
  current->rightVedge->leftVedge = newVDedge;
  current->rightVedge = newVDedge;
}

struct VoronoiEdge *VoronoiDiagramGenerator::leftVoronoiEdge(struct Point *node) {
  // linear-search of the Edgelist to find the left edge of p
  struct VoronoiEdge *index = EdgeListLeftBorder; // start from the left end
  do {
    index = index->rightVedge;
  } while(index != EdgeListRightBorder && rightCheck(index, node));
  index = index->leftVedge;
  
  return index;
}

// This delete routine can't reclaim node, since pointers from hash table may be present.  
void VoronoiDiagramGenerator::EdgeListDelete(struct VoronoiEdge *VDedge) {
  VDedge->leftVedge->rightVedge = VDedge->rightVedge;
  VDedge->rightVedge->leftVedge = VDedge->leftVedge;
  VDedge->innerVedge = (struct Edge *) DELETED;
}


/******************************************************************************
 * Geometry operations
 */

std::vector<std::pair<double, double> > VoronoiDiagramGenerator::convexHullset(std::vector<std::pair<double, double> > viewAngleList) {
  std::vector<std::pair<double, double> > boundary;
  int length = viewAngleList.size(), index = 0;
  if(length == 1) return viewAngleList;

  sort(viewAngleList.begin(), viewAngleList.end(), [](const std::pair<double, double> &a, const std::pair<double, double> &b) {
      return a.first < b.first || (a.first == b.first && a.second < b.second);
    });
  // Build lower hull
  boundary.resize(2 * length);
  for(int i = 0; i < length; i++) {
    while(index >= 2 && orientation(boundary[index-2], boundary[index-1], viewAngleList[i]) <= EPS) index++;
    boundary[index++] = viewAngleList[i];
  }
  // Build upper hull
  for(int i = length - 2, j = index+1; i >= 0; i--) {
    while(index >= j && orientation(boundary[index-2], boundary[index-1], viewAngleList[i]) <= EPS) index--;
    boundary[index++] = boundary[i];
  }

  boundary.resize(index-1);
  
  return boundary;
}

int VoronoiDiagramGenerator::orientation(std::pair<double, double> p0, std::pair<double, double> a, std::pair<double, double> b) {
  return (a.first - p0.first) * (b.second - p0.second) - (a.second - p0.second) * (b.first - p0.first);
}

std::vector<std::pair<double, double> > VoronoiDiagramGenerator::convexHullExpand(std::vector<std::pair<double, double> > chBoundary, double moat) {
  // Input boudnary points should be clockwise of counterclockwise
  if(moat < EPS || chBoundary.size() < 3) return chBoundary;
  // Find an inner point in the convex hull;
  std::pair<double, double> innerPoint;
  innerPoint.first = ((chBoundary[0].first + chBoundary[1].first)*0.5 + chBoundary[2].first)*0.5;
  innerPoint.second = ((chBoundary[0].second + chBoundary[1].second)*0.5 + chBoundary[2].second)*0.5;

  //chBoundary.push_back(chBoundary.front());
  std::vector<std::pair<double, double> > boundaryExpand;
  int length = chBoundary.size();
  for(int i = 0; i < length; i++) {
    std::pair<double, double> p1, p2, p3; // Three consecutive points
    p1 = chBoundary[i % length];
    p2 = chBoundary[(i+1) % length];
    p3 = chBoundary[(i+2) % length];
    
    double a1, b1, c1; // expanded line
    a1 = -(p2.second - p1.second);
    b1 = p2.first - p1.first;
    c1 = p1.first * (p2.second - p1.second) - p1.second * (p2.first - p1.first);

    double newc1temp = c1 - sqrt(a1*a1 + b1*b1) * moat;
    double newc2temp = c1 + sqrt(a1*a1 + b1*b1) * moat;
    if(distPointToLineABC(innerPoint, a1, b1, newc1temp) < distPointToLineABC(innerPoint, a1, b1, newc2temp)) c1 = newc2temp;
    else c1 = newc1temp;

    double a2, b2, c2; // expanded line
    a2 = -(p3.second - p2.second);
    b2 = p3.first - p2.first;
    c2 = p2.first * (p3.second - p2.second) - p2.second * (p3.first - p2.first);

    newc1temp = c2 - sqrt(a2*a2 + b2*b2) * moat;
    newc2temp = c2 + sqrt(a2*a2 + b2*b2) * moat;
    if(distPointToLineABC(innerPoint, a2, b2, newc1temp) < distPointToLineABC(innerPoint, a2, b2, newc2temp)) c2 = newc2temp;
    else c2 = newc1temp;

    std::pair<double, double> intersection;
    if((a1*(c2*b1 - c1*b2) - b1*(c2*a1 - c1*a2)) < EPS && (a1*(c2*b1 - c1*b2) - b1*(c2*a1 - c1*a2)) > -EPS) intersection.first = p2.first;
    else intersection.first = -c1 * (c2*b1 - c1*b2) / (a1*(c2*b1 - c1*b2) - b1*(c2*a1 - c1*a2));
    if((b1*(c2*a1 - c2*a2) - a1*(c2*b1 - c1*b2)) < EPS && (b1*(c2*a1 - c2*a2) - a1*(c2*b1 - c1*b2)) > -EPS) intersection.second = p2.second;
    else intersection.second = -c1 * (c2*a1 - c2*a2) / (b1*(c2*a1 - c2*a2) - a1*(c2*b1 - c1*b2));

    boundaryExpand.push_back(intersection);
  }

  return boundaryExpand;
}

double VoronoiDiagramGenerator::distpair(std::pair<double, double> s1, std::pair<double, double> s2) {
  return sqrt((s1.first - s2.first) * (s1.first - s2.first) + (s1.second - s2.second) * (s1.second - s2.second));
}

double VoronoiDiagramGenerator::distPointToLineABC(std::pair<double, double> innerPoint, double a, double b, double c) {
  return std::fabs((a * innerPoint.first + b * innerPoint.second + c)) / sqrt(a*a + b*b);
}


struct Site * VoronoiDiagramGenerator::leftSite(struct VoronoiEdge *VDedge) {
  if(VDedge->innerVedge == (struct Edge *)NULL) return bottomsite;
  return VDedge->ELpm == LEFTedge ? VDedge->innerVedge->siteSeg[LEFTedge] : VDedge->innerVedge->siteSeg[RIGHTedge];
}

struct Site *VoronoiDiagramGenerator::rightSite(struct VoronoiEdge *VDedge) {
  //if this halfedge has no edge, return the bottom site (whatever that is)
  if(VDedge->innerVedge == (struct Edge *)NULL) return bottomsite;
  //if the ELpm field is zero, return the site 0 that this edge bisects, otherwise return site number 1
  return VDedge->ELpm == LEFTedge ? VDedge->innerVedge->siteSeg[RIGHTedge] : VDedge->innerVedge->siteSeg[LEFTedge] ;
}

struct Edge * VoronoiDiagramGenerator::generateBisector(struct Site *s1, struct Site *s2) {
  struct Edge *newedge = new struct Edge();
  
  newedge->siteSeg[0] = s1; //store the sites that this edge is bisecting
  newedge->siteSeg[1] = s2;
  usedMark(s1); // mark the sites are used
  usedMark(s2);
  newedge -> bisectSeg[0] = (struct Site *) NULL; //to begin with, there are no endpoints on the bisector - it goes to infinity
  newedge -> bisectSeg[1] = (struct Site *) NULL;
  
  double dx = s2->coord.x - s1->coord.x; //get the difference in x dist between the sites
  double dy = s2->coord.y - s1->coord.y;
  double adx = dx > 0 ? dx : -dx; //make sure that the difference in positive
  double ady = dy > 0 ? dy : -dy;
  // The line function is ax + by = c
  newedge->c = (double)(s1->coord.x * dx + s1->coord.y * dy + (dx*dx + dy*dy)*0.5); //get the slope of the line

  if (adx > ady) {
    newedge->a = 1.0;
    newedge->b = dy/dx;
    newedge->c /= dx; //set formula of line, with x fixed to 1
  } else {
    newedge->b = 1.0;
    newedge->a = dx/dy;
    newedge->c /= dy; //set formula of line, with y fixed to 1
  }
  newedge->edgenbr = edgeNumber; // mark the index of this new edge
  edgeNumber += 1;
  return newedge;
}

//create a new site where the HalfEdges el1 and el2 intersect - note that the Point in the argument list is not used, don't know why it's there
struct Site * VoronoiDiagramGenerator::generateIntersection(struct VoronoiEdge *VDedge1, struct VoronoiEdge *VDedge2) {
  struct Edge *e1 = VDedge1->innerVedge;
  struct Edge *e2 = VDedge2->innerVedge;
  if(e1 == (struct Edge*) NULL || e2 == (struct Edge*) NULL) return ((struct Site *) NULL); // check whether both sides have the edge

  //if the two edges bisect the same parent (same focal site), return
  if (e1->siteSeg[1] == e2->siteSeg[1]) return ((struct Site *) NULL);
  
  double crossProduct = e1->a * e2->b - e1->b * e2->a; // cross product
  if (-EPS < crossProduct && crossProduct < EPS) return ((struct Site *) NULL);
  
  double xint = (e1->c * e2->b - e2->c * e1->b)/crossProduct;
  double yint = (e2->c * e1->a - e1->c * e2->a)/crossProduct;
  struct Edge *edge;
  struct VoronoiEdge *VDedgeSelect;
  if((e1->siteSeg[1]->coord.y < e2->siteSeg[1]->coord.y) ||
     (e1->siteSeg[1]->coord.y == e2->siteSeg[1]->coord.y && e1->siteSeg[1]->coord.x < e2->siteSeg[1]->coord.x)) {
    VDedgeSelect = VDedge1;
    edge = e1;
  } else {
    VDedgeSelect = VDedge2;
    edge = e2;
  }
  int right_of_site = xint >= edge->siteSeg[1]->coord.x;
  if ((right_of_site && VDedgeSelect->ELpm == LEFTedge) || (!right_of_site && VDedgeSelect->ELpm == RIGHTedge)) return ((struct Site *) NULL);
  
  //create a new site at the point of intersection - this is a new vector event waiting to happen
  struct Site *futureIntersectionVertex = new struct Site();
  futureIntersectionVertex->visited = 0;
  futureIntersectionVertex->coord.x = xint;
  futureIntersectionVertex->coord.y = yint;
  return futureIntersectionVertex;
}

/* returns 1 if p is to right of halfedge e */
int VoronoiDiagramGenerator::rightCheck(struct VoronoiEdge *VDedge, struct Point *node) {
  struct Edge *edge = VDedge->innerVedge;
  struct Site *topsite = edge->siteSeg[1];
  
  int right_of_site = node->x > topsite->coord.x;
  if(right_of_site && VDedge->ELpm == LEFTedge) return 1;
  if(!right_of_site && VDedge->ELpm == RIGHTedge) return 0;
  
  int fast = 0, above;
  if (edge->a == 1.0) {
    double dyp = node->y - topsite->coord.y;
	double dxp = node->x - topsite->coord.x;
	if((!right_of_site && (edge->b < 0.0)) || (right_of_site && (edge->b >= 0.0))) {
      above = dyp >= edge->b * dxp;	
      fast = above;
	} else {
      above = node->x + node->y * edge->b > edge->c;
      if(edge->b<0.0) above = !above;
      if(!above) fast = 1;
	}
	if(!fast) {
      double dxs = topsite->coord.x - edge->siteSeg[0]->coord.x;
      above = edge->b * (dxp*dxp - dyp*dyp) < dxs*dyp*(1.0+2.0*dxp/dxs + edge->b * edge->b);
      if(edge->b < 0.0) above = !above;
	}
  } else { //edge->b == 1.0 
    double yl = edge->c - edge->a * node->x;
	double t1 = node->y - yl;
	double t2 = node->x - topsite->coord.x;
	double t3 = yl - topsite->coord.y;
	above = t1*t1 > t2*t2 + t3*t3;
  }
  return (VDedge->ELpm == LEFTedge ? above : !above);
}


void VoronoiDiagramGenerator::endPoint(struct Edge *edge, int pos, struct Site *st) {
  edge->bisectSeg[pos] = st;
  usedMark(st);
  if(edge->bisectSeg[RIGHTedge - pos]== (struct Site *) NULL) return;

  VDedgeGenerator(edge);

  unusedMark(edge->siteSeg[LEFTedge]);
  unusedMark(edge->siteSeg[RIGHTedge]);
}


double VoronoiDiagramGenerator::distance(struct Site *s1, struct Site *s2) {
  double dx, dy;
  dx = s1->coord.x - s2->coord.x;
  dy = s1->coord.y - s2->coord.y;
  return (double)(sqrt(dx*dx + dy*dy));
}


void VoronoiDiagramGenerator::generateVDvertex(struct Site *st) {
  st->sitemarker = VDverticesNumber;
  VDverticesNumber += 1;
}


void VoronoiDiagramGenerator::unusedMark(struct Site *st) {
  st->visited -= 1;
}

void VoronoiDiagramGenerator::usedMark(struct Site *st) {
  st->visited += 1;
}

/******************************************************************************
 * Priority queue for the sites and intersections on the plane
 */
bool VoronoiDiagramGenerator::PQinit(){
  PQsize = 0;
  //PQtablesize = 4 * sqrt_siteNumber;
  if(siteNumber == 0) return false;
  //PQtable.resize(PQtablesize);
  PQtable = new struct VoronoiEdge();

  return true;
}

int VoronoiDiagramGenerator::PQempty() {
  return PQsize == 0;
}

// push the ArcEdge in the PQtable
void VoronoiDiagramGenerator::PQinsert(struct VoronoiEdge *current, struct Site * v, double offset) {
  current->vertex = v;
  usedMark(v); // v is used
  current->ystar = (double)(v->coord.y + offset); // mapping *(z)
  struct VoronoiEdge *last = PQtable;
  struct VoronoiEdge *next;
  // Find the right position for current to keep the order
  while ((next = last->PQnext) != (struct VoronoiEdge *) NULL &&
         (current->ystar > next->ystar ||
          (current->ystar == next->ystar && v->coord.x > next->vertex->coord.x))) {
    last = next;
  };
  current->PQnext = last->PQnext;
  last->PQnext = current;
  PQsize += 1;
}

// Remove the ArcEdge PQtable
void VoronoiDiagramGenerator::PQdelete(struct VoronoiEdge *VDedge) {
  if(VDedge->vertex != (struct Site *) NULL) {
    struct VoronoiEdge *index = PQtable;
    
    while (index->PQnext != VDedge) index = index->PQnext;
    index->PQnext = VDedge->PQnext;
    PQsize -= 1;
    unusedMark(VDedge->vertex);
    VDedge->vertex = (struct Site *) NULL;
  }
}

struct VoronoiEdge *VoronoiDiagramGenerator::PQpopMin() {
  struct VoronoiEdge *curr = PQtable->PQnext;
  PQtable->PQnext = curr->PQnext;
  PQsize -= 1;
  return curr;
}

struct Point VoronoiDiagramGenerator::PQtop() {
  struct Point minPoint;
  
  minPoint.x = PQtable->PQnext->vertex->coord.x;
  minPoint.y = PQtable->PQnext->ystar;
  return minPoint;
}


/********************************************************************
 * Output preparation
 */

void VoronoiDiagramGenerator::VDedgeGenerator(struct Edge *VDedge) {
  //double temp = 0;;
  double x1 = VDedge->siteSeg[0]->coord.x;
  double x2 = VDedge->siteSeg[1]->coord.x;
  double y1 = VDedge->siteSeg[0]->coord.y;
  double y2 = VDedge->siteSeg[1]->coord.y;

  // Ignore the Voronoi Diagram edge is two sites are too close
  if(sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1))) < gapThreshold) return;
  
  double pxmin = borderMinX;
  double pxmax = borderMaxX;
  double pymin = borderMinY;
  double pymax = borderMaxY;

  struct Site *s1, *s2; // Voronoi Diagram vertices
  if(VDedge->a == 1.0 && VDedge->b >= 0.0) {
    s1 = VDedge->bisectSeg[1];
    s2 = VDedge->bisectSeg[0];
  } else {
    s1 = VDedge->bisectSeg[0];
    s2 = VDedge->bisectSeg[1];
  }
  
  if(VDedge->a == 1.0) {
    y1 = pymin;
    if (s1 != (struct Site *)NULL && s1->coord.y > pymin) y1 = s1->coord.y;
    if(y1 > pymax) y1 = pymax;
    x1 = VDedge->c - VDedge->b * y1;
    
    y2 = pymax;
    if (s2 != (struct Site *)NULL && s2->coord.y < pymax) y2 = s2->coord.y;
    if(y2 < pymin) y2 = pymin;
    x2 = VDedge->c - VDedge->b * y2;
    
    //if (((x1 > pxmax) & (x2 > pxmax)) | ((x1 < pxmin) & (x2 < pxmin))) return;
    if (((x1 > pxmax) && (x2 > pxmax)) || ((x1 < pxmin) && (x2 < pxmin))) return;
    
    if(x1 > pxmax) {
      x1 = pxmax;
      y1 = (VDedge->c - x1)/VDedge->b;
    }
    if(x1 < pxmin) {
      x1 = pxmin;
      y1 = (VDedge->c - x1)/VDedge->b;
    }
    if(x2 > pxmax) {
      x2 = pxmax;
      y2 = (VDedge->c - x2)/VDedge->b;
    }
    if(x2 < pxmin) {
      x2 = pxmin;
      y2 = (VDedge->c - x2)/VDedge->b;
    }
  } else {
    x1 = pxmin;
    if (s1 != (struct Site *)NULL && s1->coord.x > pxmin) x1 = s1->coord.x;
    if(x1 > pxmax) x1 = pxmax;
    y1 = VDedge->c - VDedge->a * x1;

    x2 = pxmax;
    if (s2 != (struct Site *)NULL && s2->coord.x < pxmax) x2 = s2->coord.x;
    if(x2 < pxmin) x2 = pxmin;
    y2 = VDedge->c - VDedge->a * x2;
    
    //if (((y1 > pymax) & (y2 > pymax)) | ((y1 < pymin) & (y2 < pymin))) return;
    if (((y1 > pymax) && (y2 > pymax)) || ((y1 < pymin) && (y2 < pymin))) return;
    
    if(y1 > pymax) {
      y1 = pymax;
      x1 = (VDedge->c - y1)/VDedge->a;
    }
    if(y1 < pymin) {
      y1 = pymin;
      x1 = (VDedge->c - y1)/VDedge->a;
    }
    if(y2 > pymax) {
      y2 = pymax;
      x2 = (VDedge->c - y2)/VDedge->a;
    }
    if(y2 < pymin) {
      y2 = pymin;
      x2 = (VDedge->c - y2)/VDedge->a;
    }
  }
  pushGraphEdge(x1, y1, x2, y2);

  std::pair<struct Point, struct Point> add;
  add.first.x = x1;
  add.first.y = y1;
  add.second.x = x2;
  add.second.y = y2;
  VDedgeSet.push_back(add);
  std::pair<int, int> siteNum;
  siteNum.first = VDedge->siteSeg[0]->sitemarker;
  siteNum.second = VDedge->siteSeg[1]->sitemarker;
  VDedgeSetHelper.push_back(siteNum);
}


/* There is an important mapping here: *(z)=(z_x, z_y+d(z)) where d(z) is the
 * Euclidean distance from z to the nearest site.
 * The edges, regions, vertices are under such a mapping.
 */
bool VoronoiDiagramGenerator::voronoiOperation() {
  // Initialize the priority queue of VoronoiEdge.
  PQinit();
  // Build the bottom (basic) site based on the 1st incoming site.
  bottomsite = nextone();
  // Initialize the hashTable of Edges.
  bool retval = EdgeListInit();
  if(!retval) return false; 
	
  struct Site *incomingSite = nextone(); // Build the new site to process
  
  while(1) {
    struct Point newPopPoint;
    if(!PQempty()) newPopPoint = PQtop(); // Find the lowest point (Geometric) in PQtable
    /* Site mission:
     * If the lowest site has a smaller y value than the lowest
     * vector intersection, process the site */
    if (incomingSite != (struct Site *) NULL &&
        (PQempty() || incomingSite->coord.y < newPopPoint.y ||
         (incomingSite->coord.y == newPopPoint.y &&
          incomingSite->coord.x < newPopPoint.x))) {
      /* Find the region (two VoronoiEdges) contineing the incomingsite */
      struct VoronoiEdge *leftHalfEdge = leftVoronoiEdge(&(incomingSite->coord)); //Get the left HalfEdge of the incomingSite
      struct VoronoiEdge *rightHalfEdge = leftHalfEdge->rightVedge; //get the first HalfEdge to the RIGHT of the new site
      /* Create Bisector */
      struct Site *bot = rightSite(leftHalfEdge); //if this halfedge has no edge, , bot = bottom site (whatever that is)
      struct Edge *siteEdge = generateBisector(bot, incomingSite); //create a new edge that bisects
      
      struct VoronoiEdge *bisector = HEcreate(siteEdge, LEFTedge); //create a new HalfEdge, setting its ELpm field to 0
      /* Update EdgeList */
      EdgeListInsert(leftHalfEdge, bisector); //insert this new bisector edge between the left and right vectors in a linked list
      struct Site *intersectionVertex = generateIntersection(leftHalfEdge, bisector);
      if (intersectionVertex != (struct Site *) NULL) { //if the new bisector intersects with the left edge, remove the left edge's vertex, and put in the new one
        PQdelete(leftHalfEdge);
        PQinsert(leftHalfEdge, intersectionVertex, distance(intersectionVertex, incomingSite));
      }
      
      leftHalfEdge = bisector; // move lbnd to be the new created VoronoiEdge, which is the leftside of e.
      
      bisector = HEcreate(siteEdge, RIGHTedge); //create a new HalfEdge, setting its ELpm field to 1
      EdgeListInsert(leftHalfEdge, bisector); //insert the new HE to the right of the original bisector earlier in the IF stmt
      intersectionVertex = generateIntersection(bisector, rightHalfEdge);
      if (intersectionVertex != (struct Site *) NULL) { //if this new bisector intersects with the
        PQinsert(bisector, intersectionVertex, distance(intersectionVertex, incomingSite)); //push the HE into the ordered linked list of vertices
      }
      incomingSite = nextone(); // move to the next site of intersection.
    }
    /* Voronoi Diagram vertex (intersection) mission */
    else if (!PQempty()) { //intersection is smallest - this is a vector event 
      struct VoronoiEdge *VDvertex = PQpopMin(); //pop the HalfEdge with the lowest vector off the ordered list of vectors
      struct VoronoiEdge *leftHalfEdgeVDv = VDvertex->leftVedge; //get the HalfEdge to the left of the above HE
      struct VoronoiEdge *rightHalfEdgeVDv = VDvertex->rightVedge; //get the HalfEdge to the right of the above HE
      struct VoronoiEdge *RrightHalfEdgeVDv = rightHalfEdgeVDv->rightVedge; //get the HalfEdge to the right of the HE to the right of the lowest HE 
      struct Site *bot = leftSite(VDvertex); //get the Site to the left of the left HE which it bisects
      struct Site *top = rightSite(rightHalfEdgeVDv); //get the Site to the right of the right HE which it bisects

      struct Site *st = VDvertex->vertex; //get the vertex that caused this event
      generateVDvertex(st); //set the vertex number - couldn't do this earlier since we didn't know when it would be processed
      endPoint(VDvertex->innerVedge, VDvertex->ELpm, st); //set the endpoint of the left HalfEdge to be this vector
      endPoint(rightHalfEdgeVDv->innerVedge,rightHalfEdgeVDv->ELpm, st); //set the endpoint of the right HalfEdge to be this vector
      EdgeListDelete(VDvertex); //mark the lowest HE for deletion - can't delete yet because there might be pointers to it in Hash Map   
      PQdelete(rightHalfEdgeVDv); //remove all vertex events to do with the  right HE
      EdgeListDelete(rightHalfEdgeVDv); //mark the right HE for deletion - can't delete yet because there might be pointers to it in Hash Map	
      int pm = LEFTedge; //set the pm variable to zero
      if (bot->coord.y > top->coord.y) { //if the site to the left of the event is higher than the Site to the right of it, then swap them and set the 'pm' variable to 1
        std::swap(top, bot);
        pm = RIGHTedge;
      }
      struct Edge *bottopEdge = generateBisector(bot, top); //create an Edge (or line) that is between the two Sites. This creates the formula of the line, and assigns a line number to it
      struct VoronoiEdge *bisector = HEcreate(bottopEdge, pm); //create a HE from the Edge 'e', and make it point to that edge with its innerVedge field
      EdgeListInsert(leftHalfEdgeVDv, bisector); //insert the new bisector to the right of the left HE
      endPoint(bottopEdge, RIGHTedge - pm, st); //set one endpoint to the new edge to be the vector point 'v'.
      //If the site to the left of this bisector is higher than the right
      //Site, then this endpoint is put in position 0; otherwise in pos 1
      unusedMark(st); //delete the vector 'v'

      //if left HE and the new bisector don't intersect, then delete the left HE, and reinsert it
      struct Site *intersectionVertex = generateIntersection(leftHalfEdgeVDv, bisector);
      if(intersectionVertex != (struct Site *) NULL) {	
        PQdelete(leftHalfEdgeVDv);
        PQinsert(leftHalfEdgeVDv, intersectionVertex, distance(intersectionVertex, bot));
      }
      //if right HE and the new bisector don't intersect, then reinsert it
      intersectionVertex = generateIntersection(bisector, RrightHalfEdgeVDv);
      if (intersectionVertex != (struct Site *) NULL) {	
        PQinsert(bisector, intersectionVertex, distance(intersectionVertex, bot));
      }
      
    }
    else break;
  }

  for(struct VoronoiEdge *it = EdgeListLeftBorder->rightVedge; it != EdgeListRightBorder; it = it->rightVedge) {
    VDedgeGenerator(it->innerVedge);
  }

  return true;	
}





