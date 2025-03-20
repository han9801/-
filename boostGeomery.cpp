#include <QApplication>
#include <QPainterPath>
#include <vector>
#include <QPainterPath>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <iostream>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// 定义点、多边形和索引节点
using point = bg::model::d2::point_xy<double>;
using polygon = bg::model::polygon<point>;
using box = bg::model::box<point>;
using value = std::pair<box, size_t>;

polygon QPainterPathToPolygon(const QPainterPath& path) {
    polygon poly;
    for (int i = 0; i < path.elementCount(); ++i) {
        auto elem = path.elementAt(i);
        bg::append(poly.outer(), point(elem.x, elem.y));
    }

    // 确保多边形闭合
    //if (!path.isClosed()) {
    //    auto first = path.elementAt(0);
    //    bg::append(poly.outer(), point(first.x, first.y));
    //}

    return poly;
}
bgi::rtree<value, bgi::quadratic<16>> buildRTree(const std::vector<QPainterPath>& paths) {
    bgi::rtree<value, bgi::quadratic<16>> rtree;

    // 遍历每条路径，生成多边形和边界框，存入R-tree
    for (size_t i = 0; i < paths.size(); ++i) {
        polygon poly = QPainterPathToPolygon(paths[i]);
        box boundingBox = bg::return_envelope<box>(poly);
        rtree.insert(std::make_pair(boundingBox, i));
    }

    return rtree;
}
void findIntersectingPathsRTree(const std::vector<QPainterPath>& group1,
                                const std::vector<QPainterPath>& group2) {
    auto rtree = buildRTree(group2);

    for (size_t i = 0; i < group1.size(); ++i) {
        polygon poly1 = QPainterPathToPolygon(group1[i]);
        box boundingBox1 = bg::return_envelope<box>(poly1);

        // R-tree快速查询可能相交的路径
        std::vector<value> result;
        rtree.query(bgi::intersects(boundingBox1), std::back_inserter(result));

        // 检测真正的几何相交
        for (const auto& item : result) {
            polygon poly2 = QPainterPathToPolygon(group2[item.second]);
            if (bg::intersects(poly1, poly2)) {
                std::cout << "路径 " << i << " 和路径 " << item.second << " 确实相交!" << std::endl;
            }
        }
    }
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 定义第一组路径
    QPainterPath path1;
    path1.addRect(0, 0, 100, 100);

    QPainterPath path2;
    path2.addRect(50, 50, 100, 100);

    // 第二组路径
    QPainterPath path3;
    path3.addEllipse(60, 60, 50, 50);

    QPainterPath path4;
    path4.addRect(200, 200, 50, 50);

    // 组织两组路径
    std::vector<QPainterPath> group1 = {path1, path2};
    std::vector<QPainterPath> group2 = {path3, path4};

    // 执行相交检测
    findIntersectingPathsRTree(group1, group2);

    return app.exec();
}
