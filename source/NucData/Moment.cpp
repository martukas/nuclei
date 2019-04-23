#include <NucData/Moment.h>

Moment::Moment(const Moment &p)
  : moment_(p.moment_)
{}

Moment::Moment(const Uncert &v)
  : moment_(v)
{}

bool Moment::valid() const
{
  return moment_.hasFiniteValue();
}

const std::string Moment::to_string() const
{
  return moment_.to_string(false);
}

const std::string Moment::to_markup() const
{
  return moment_.to_markup();
}

void Moment::add_reference(std::string ref)
{
  references_.push_back(ref);
}

void Moment::set_references(std::vector<std::string> refs)
{
  references_ = refs;
}
