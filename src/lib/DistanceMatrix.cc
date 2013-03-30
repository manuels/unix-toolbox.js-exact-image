#include <math.h>
#include "DistanceMatrix.hh"

class QueueElement
{
public:
  int x;
  int y;
  unsigned int xdiff;
  unsigned int ydiff;

  QueueElement(unsigned int i_x, unsigned int i_y)
  {
    x=i_x;
    y=i_y;
    xdiff=0;
    ydiff=0;
  }

  QueueElement(const QueueElement& e, int direction)
  {
    switch (direction) {
    case 0:
      x=e.x-1;
      y=e.y;
      xdiff=e.xdiff-1;
      ydiff=e.ydiff;
      break;
    case 1:
      x=e.x;
      y=e.y-1;
      xdiff=e.xdiff;
      ydiff=e.ydiff-1;
      break;
    case 2:
      x=e.x+1;
      y=e.y;
      xdiff=e.xdiff+1;
      ydiff=e.ydiff;
      break;
    default:
      x=e.x;
      y=e.y+1;
      xdiff=e.xdiff;
      ydiff=e.ydiff+1;
    }
  }
      
  unsigned int Value() const
  {
    return ((xdiff*xdiff) + (ydiff*ydiff)); 
  }
};


DistanceMatrix::DistanceMatrix(Image& image, unsigned int fg_threshold)
  : DataMatrix<unsigned int>(image.w, image.h)
{
  Queue queue;
  Init(queue);
  int line=0, row=0;
  Image::iterator i=image.begin();
  Image::iterator end=image.end();
  for (; i!=end ; ++i) {
    if ((*i).getL() < fg_threshold) {
      queue.push_back(QueueElement(row, line));
      data[row][line]=0;
    }

    if (++row == image.w) {
      line++;
      row=0;
    }
  }

  RunBFS(queue);
}



DistanceMatrix::DistanceMatrix(const FGMatrix& image)
  : DataMatrix<unsigned int>(image.w, image.h)
{
  Queue queue;
  Init(queue);
  for (unsigned int x=0; x<w; x++)
    for (unsigned int y=0; y<h ; y++)
      if (image(x,y)) {
	queue.push_back(QueueElement(x, y));
	data[x][y]=0;
      }

  RunBFS(queue);
}



void DistanceMatrix::Init(Queue& queue)
{
  for (unsigned int x=0; x<w; x++)
    for (unsigned int y=0; y<h; y++)
      data[x][y]=undefined_dist;

  queue.reserve(4*w*h);
}

void DistanceMatrix::RunBFS(Queue& queue)
{
  unsigned int pos=0;
  while (pos < queue.size()) {
    for (unsigned int direction=0; direction<4; direction++) {
      queue.push_back(QueueElement(queue[pos],direction));
      QueueElement& last=queue.back();
      unsigned int value=last.Value();
      if (last.x < 0 || last.x >= (int)w || last.y < 0 || last.y >= (int)h || value >= data[last.x][last.y])
	queue.pop_back();
      else
	data[last.x][last.y]=value;
    }
    pos++;
  }

  for (unsigned int x=0; x<w; x++)
    for (unsigned int y=0; y<h; y++)
      data[x][y]=(unsigned int) sqrt((double) (data[x][y] << 2*precission_shift));
  queue.clear();
}

DistanceMatrix::DistanceMatrix(const DistanceMatrix& source, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
  : DataMatrix<unsigned int>(source, x,y,w,h)
{
}
  
DistanceMatrix::~DistanceMatrix()
{
}

