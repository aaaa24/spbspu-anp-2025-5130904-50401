#include <iostream>
#include <cmath>

namespace chernov {
  struct point_t {
    double x, y;
  };

  struct rectangle_t {
    double width, height;
    point_t pos;
  };

  struct Shape {
    virtual ~Shape() = default;
    virtual double getArea() const = 0;
    virtual rectangle_t getFrameRect() const = 0;
    virtual void move(point_t p) = 0;
    virtual void move(double dx, double dy) = 0;
    virtual void scale(double k) = 0;
  };

  struct Rectangle: Shape {
    Rectangle(double a, double b, point_t o);
    double getArea() const override;
    rectangle_t getFrameRect() const override;
    void move(point_t p) override;
    void move(double dx, double dy) override;
    void scale(double k) override;
    double side_x, side_y;
    point_t center;
  };

  struct Polygon: Shape {
    Polygon(point_t * points, size_t size);
    ~Polygon() override;
    Polygon(const Polygon & p);
    Polygon & operator=(const Polygon & p);
    Polygon(Polygon && p);
    Polygon & operator=(Polygon && p);
    double getArea() const override;
    rectangle_t getFrameRect() const override;
    void move(point_t p) override;
    void move(double dx, double dy) override;
    void scale(double k) override;
    double getSignedArea() const;
    point_t getCentroid() const;
    point_t * verts;
    size_t count;
    point_t center;
  };

  struct Bubble: Shape {
    Bubble(double r, point_t o, point_t a);
    double getArea() const override;
    rectangle_t getFrameRect() const override;
    void move(point_t p) override;
    void move(double dx, double dy) override;
    void scale(double k) override;
    double radius;
    point_t center, anchor;
  };

  void scaleByPoint(Shape ** shapes, size_t count, double k, point_t p);
  rectangle_t getTotalFrameRect(const Shape * const * shapes, size_t count);
  std::ostream & printShapeInfo(std::ostream & out, const Shape * shape, const char * name);
  std::ostream & printShapesInfo(std::ostream & out, const Shape * const * shapes, const char ** names, size_t count);
}

int main()
{
  using namespace chernov;

  std::ostream & output = std::cout;
  int result = 0;

  const size_t count = 5;
  Shape * shapes[count];
  const char * names[count];

  shapes[0] = new Rectangle(5, 6, {1, 2});
  names[0] = "Rectangle 1";

  shapes[1] = new Rectangle(10, 2, {-10, 3});
  names[1] = "Rectangle 2";

  point_t points1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
  size_t points_count = 4;
  shapes[2] = new Polygon(points1, points_count);
  names[2] = "Polygon 1";

  point_t points2[] = {{0, 0}, {4, 1}, {5, 4}, {5, 8}, {4, 10}, {3, 8}, {2, 5}, {-1, 1}};
  points_count = 8;
  shapes[3] = new Polygon(points2, points_count);
  names[3] = "Polygon 2";

  shapes[4] = new Bubble(10, {0, 0}, {2, 2});
  names[4] = "Bubble";

  printShapesInfo(output, shapes, names, count);

  output << "\n\nEnter x, y and k: ";

  double x = 0, y = 0, k = 0;
  while (std::cin >> x >> y >> k) {
    if (k <= 0) {
      std::cerr << "k cannot be less than or equal to zero\n";
      result = 1;
      break;
    }
    scaleByPoint(shapes, count, k, {x, y});
    output << "\n\n";
    printShapesInfo(output, shapes, names, count);
    output << "\n\nEnter x, y and k: ";
  }

  if (std::cin.fail() && (!std::cin.eof() || k == 0)) {
    std::cerr << "bad input\n";
    result = 1;
  }

  for (size_t i = 0; i < count; ++i) {
    delete shapes[i];
  }
  return result;
}

void chernov::scaleByPoint(Shape ** shapes, size_t count, double k, point_t p)
{
  if (k <= 0) {
    throw std::invalid_argument("k must be positive");
  }
  for (size_t i = 0; i < count; ++i) {
    Shape * shape = shapes[i];
    point_t first_pos = shape->getFrameRect().pos;
    shape->move(p);
    point_t second_pos = shape->getFrameRect().pos;
    double dx = k * (first_pos.x - second_pos.x);
    double dy = k * (first_pos.y - second_pos.y);
    shape->move(dx, dy);
    shape->scale(k);
  }
}

chernov::rectangle_t chernov::getTotalFrameRect(const Shape * const * shapes, size_t count)
{
  rectangle_t frame = shapes[0]->getFrameRect();
  double min_x = frame.pos.x - frame.width / 2;
  double min_y = frame.pos.y - frame.height / 2;
  double max_x = min_x + frame.width;
  double max_y = min_y + frame.height;
  for (size_t i = 1; i < count; ++i) {
    frame = shapes[i]->getFrameRect();
    min_x = std::min(min_x, frame.pos.x - frame.width / 2);
    min_y = std::min(min_y, frame.pos.y - frame.height / 2);
    max_x = std::max(max_x, frame.pos.x + frame.width / 2);
    max_y = std::max(max_y, frame.pos.y + frame.height / 2);
  }
  double width = max_x - min_x;
  double height = max_y - min_y;
  point_t pos = {min_x + width / 2, min_y + height / 2};
  return {width, height, pos};
}

std::ostream & chernov::printShapeInfo(std::ostream & out, const Shape * shape, const char * name)
{
  out << name << ":\n";
  out << "  area: " << shape->getArea() << "\n";
  rectangle_t frame = shape->getFrameRect();
  out << "  frame rectangle:\n";
  out << "    width: " << frame.width << "\n";
  out << "    height: " << frame.height << "\n";
  out << "    position: (" << frame.pos.x << "; " << frame.pos.y << ")\n";
  return out;
}

std::ostream & chernov::printShapesInfo(std::ostream & out, const Shape * const * shapes, const char ** names, size_t count)
{
  double total_area = 0;
  for (size_t i = 0; i < count; ++i) {
    printShapeInfo(out, shapes[i], names[i]) << "\n";
    total_area += shapes[i]->getArea();
  }
  out << "Total area: " << total_area << "\n";
  rectangle_t frame = getTotalFrameRect(shapes, count);
  out << "Total frame rectangle:\n";
  out << "  width: " << frame.width << "\n";
  out << "  height: " << frame.height << "\n";
  out << "  position: (" << frame.pos.x << "; " << frame.pos.y << ")\n";
  return out;
}

chernov::Rectangle::Rectangle(double a, double b, point_t o):
  Shape(),
  side_x(a),
  side_y(b),
  center(o)
{
  if (side_x <= 0 || side_y <= 0) {
    throw std::invalid_argument("the side must be greater than 0");
  }
}

double chernov::Rectangle::getArea() const
{
  return side_x * side_y;
}

chernov::rectangle_t chernov::Rectangle::getFrameRect() const
{
  return {side_x, side_y, center};
}

void chernov::Rectangle::move(point_t p)
{
  center = p;
}

void chernov::Rectangle::move(double dx, double dy)
{
  move({center.x + dx, center.y + dy});
}

void chernov::Rectangle::scale(double k)
{
  side_x *= k;
  side_y *= k;
}

chernov::Polygon::Polygon(point_t * points, size_t size):
  Shape(),
  verts(new point_t[size]),
  count(size),
  center({0, 0})
{
  if (count < 3) {
    throw std::invalid_argument("the count must not be less than 3");
  }
  for (size_t i = 0; i < count; ++i) {
    verts[i] = points[i];
  }
  center = getCentroid();
}

chernov::Polygon::~Polygon()
{
  delete [] verts;
}

chernov::Polygon::Polygon(const Polygon & p):
  Shape(),
  verts(new point_t[p.count]),
  count(p.count),
  center(p.center)
{
  for (size_t i = 0; i < count; ++i) {
    verts[i] = p.verts[i];
  }
}

chernov::Polygon & chernov::Polygon::operator=(const Polygon & p)
{
  if (this == &p) {
    return *this;
  }
  point_t * points = new point_t[p.count];
  for (size_t i = 0; i < p.count; ++i) {
    points[i] = p.verts[i];
  }
  delete [] verts;
  verts = points;
  count = p.count;
  center = p.center;
  return *this;
}

chernov::Polygon::Polygon(Polygon && p):
  Shape(),
  verts(p.verts),
  count(p.count),
  center(p.center)
{
  p.verts = nullptr;
}

chernov::Polygon & chernov::Polygon::operator=(Polygon && p)
{
  if (this == &p) {
    return *this;
  }
  delete [] verts;
  verts = p.verts;
  count = p.count;
  center = p.center;
  p.verts = nullptr;
  return *this;
}

double chernov::Polygon::getSignedArea() const
{
  double area = 0;
  for (size_t i = 0; i < count; ++i) {
    area += verts[i].x * verts[(i + 1) % count].y;
    area -= verts[i].y * verts[(i + 1) % count].x;
  }
  area *= 0.5;
  return area;
}

double chernov::Polygon::getArea() const
{
  return std::abs(getSignedArea());
}

chernov::rectangle_t chernov::Polygon::getFrameRect() const
{
  double min_x = verts[0].x, min_y = verts[0].y;
  double max_x = min_x, max_y = min_y;
  for (size_t i = 0; i < count; ++i) {
    min_x = std::min(min_x, verts[i].x);
    min_y = std::min(min_y, verts[i].y);
    max_x = std::max(max_x, verts[i].x);
    max_y = std::max(max_y, verts[i].y);
  }
  double width = max_x - min_x;
  double height = max_y - min_y;
  point_t pos = {min_x + width / 2, min_y + height / 2};
  return {width, height, pos};
}

void chernov::Polygon::move(point_t p)
{
  double dx = p.x - center.x;
  double dy = p.y - center.y;
  move(dx, dy);
}

void chernov::Polygon::move(double dx, double dy)
{
  for (size_t i = 0; i < count; ++i) {
    verts[i].x += dx;
    verts[i].y += dy;
  }
  center.x += dx;
  center.y += dy;
}

void chernov::Polygon::scale(double k)
{
  for (size_t i = 0; i < count; ++i) {
    double dx = k * (verts[i].x - center.x);
    double dy = k * (verts[i].y - center.y);
    verts[i].x = center.x + dx;
    verts[i].y = center.y + dy;
  }
}

chernov::point_t chernov::Polygon::getCentroid() const
{
  double cx = 0.0, cy = 0.0;
  for (size_t i = 0; i < count; ++i) {
    double x0 = verts[i].x, y0 = verts[i].y;
    double x1 = verts[(i + 1) % count].x, y1 = verts[(i + 1) % count].y;
    double cross = x0 * y1 - x1 * y0;
    cx += (x0 + x1) * cross;
    cy += (y0 + y1) * cross;
  }
  double signed_area = getSignedArea();
  cx /= 6 * signed_area;
  cy /= 6 * signed_area;
  return {cx, cy};
}

chernov::Bubble::Bubble(double r, point_t o, point_t a):
  radius(r),
  center(o),
  anchor(a)
{
  if ((o.x - a.x) * (o.x - a.x) + (o.y - a.y) * (o.y - a.y) > r * r) {
    throw std::invalid_argument("the anchor must be inside the circle");
  }
  if (o.x == a.x && o.y == a.y) {
    throw std::invalid_argument("the anchor must not be equal to the center");
  }
  if (r <= 0) {
    throw std::invalid_argument("the radius must be greater than 0");
  }
}

double chernov::Bubble::getArea() const
{
  const double PI = acos(-1.0);
  double area = PI * radius * radius;
  return area;
}

chernov::rectangle_t chernov::Bubble::getFrameRect() const
{
  double size = 2 * radius;
  return {size, size, center};
}

void chernov::Bubble::move(point_t p)
{
  double dx = p.x - anchor.x;
  double dy = p.y - anchor.y;
  move(dx, dy);
}

void chernov::Bubble::move(double dx, double dy)
{
  anchor.x += dx;
  anchor.y += dy;
  center.x += dx;
  center.y += dy;
}

void chernov::Bubble::scale(double k)
{
  radius *= k;
  double dx = center.x - anchor.x;
  double dy = center.y - anchor.y;
  center.x = anchor.x + k * dx;
  center.y = anchor.y + k * dy;
}
