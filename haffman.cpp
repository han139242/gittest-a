#include "haffman.h"

#include <utility>


class haffmanData : public QSharedData
{
public:

};

haffman::haffman() : data(new haffmanData)
{

}

haffman::haffman(const haffman &rhs)
    : data{rhs.data}
{

}

haffman::haffman(haffman &&rhs)
    : data{std::move(rhs.data)}
{

}

haffman &haffman::operator=(const haffman &rhs)
{
    if (this != &rhs)
        data = rhs.data;
    return *this;
}

haffman &haffman::operator=(haffman &&rhs)
{
    if (this != &rhs)
        data = std::move(rhs.data);
    return *this;
}

haffman::~haffman()
{

}
