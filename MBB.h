//
// Created by lojaz on 13/10/2022.
//

#ifndef LINES_MBB_H
#define LINES_MBB_H
#include <SFML/Graphics.hpp>
#include <cmath>
using namespace sf;
struct MBB
{
    //Attributes
    Vector2f topLeft, topRight, bottomRight, bottomLeft;
    Color color = Color::White;
    //Constructors
    MBB() = default;
    MBB(Vector2f topL, Vector2f bottomR)
    {
        topLeft = topL;
        bottomRight = bottomR;
        topRight = {bottomRight.x, topLeft.y};
        bottomLeft = {topLeft.x, bottomRight.y};
    }
    //methods
    float perimeter() const{
        auto l = abs(bottomRight.x - topLeft.x);
        auto w = abs(bottomRight.y - topLeft.y);
        return 2*(l+w);
    }

    float area() const{
        auto l = bottomRight.x - topLeft.x;
        auto w = bottomRight.y - topLeft.y;
        return std::abs(l*w);
    }

    void merge(MBB other){
        topLeft = {std::min(topLeft.x, other.topLeft.x),
                   std::min(topLeft.y, other.topLeft.y)};
        bottomRight = {std::min(bottomRight.x, other.bottomRight.x),
                   std::min(bottomRight.y, other.bottomRight.y)};
    }


    void draw(RenderWindow& window){
        std::vector<Vector2f> points = {topLeft, topRight, bottomRight, bottomLeft, topLeft};
        for(int i = 1; i < points.size(); ++i){
            sf::Vertex vertices[2] = {Vertex(Vector2f(points[i - 1]), color), Vertex(points[i], color)};
            window.draw(vertices, 2, sf::Lines);
        }
    }
};


#endif //LINES_MBB_H
