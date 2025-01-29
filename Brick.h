#ifndef BRICK_H
#define BRICK_H

class Brick {
private:
    int weight;

public:
    explicit Brick(int weight);
    int getWeight() const;
};

#endif // BRICK_H