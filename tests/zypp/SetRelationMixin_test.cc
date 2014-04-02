#include <boost/test/auto_unit_test.hpp>
#include "zypp/base/SetRelationMixin.h"

using zypp::SetCompare;
using zypp::SetRelation;

BOOST_AUTO_TEST_CASE(set_compare)
{
  BOOST_CHECK( SetCompare::uncomparable		== SetCompare::uncomparable );
  BOOST_CHECK( SetCompare::uncomparable		!= SetCompare::equal );
  BOOST_CHECK( SetCompare::uncomparable		!= SetCompare::properSubset );
  BOOST_CHECK( SetCompare::uncomparable		!= SetCompare::properSuperset );
  BOOST_CHECK( SetCompare::uncomparable		!= SetCompare::disjoint );

  BOOST_CHECK( SetCompare::equal		!= SetCompare::uncomparable );
  BOOST_CHECK( SetCompare::equal		== SetCompare::equal );
  BOOST_CHECK( SetCompare::equal		!= SetCompare::properSubset );
  BOOST_CHECK( SetCompare::equal		!= SetCompare::properSuperset );
  BOOST_CHECK( SetCompare::equal		!= SetCompare::disjoint );

  BOOST_CHECK( SetCompare::properSubset		!= SetCompare::uncomparable );
  BOOST_CHECK( SetCompare::properSubset		!= SetCompare::equal );
  BOOST_CHECK( SetCompare::properSubset		== SetCompare::properSubset );
  BOOST_CHECK( SetCompare::properSubset		!= SetCompare::properSuperset );
  BOOST_CHECK( SetCompare::properSubset		!= SetCompare::disjoint );

  BOOST_CHECK( SetCompare::properSuperset	!= SetCompare::uncomparable );
  BOOST_CHECK( SetCompare::properSuperset	!= SetCompare::equal );
  BOOST_CHECK( SetCompare::properSuperset	!= SetCompare::properSubset );
  BOOST_CHECK( SetCompare::properSuperset	== SetCompare::properSuperset );
  BOOST_CHECK( SetCompare::properSuperset	!= SetCompare::disjoint );

  BOOST_CHECK( SetCompare::disjoint		!= SetCompare::uncomparable );
  BOOST_CHECK( SetCompare::disjoint		!= SetCompare::equal );
  BOOST_CHECK( SetCompare::disjoint		!= SetCompare::properSubset );
  BOOST_CHECK( SetCompare::disjoint		!= SetCompare::properSuperset );
  BOOST_CHECK( SetCompare::disjoint		== SetCompare::disjoint );
}

BOOST_AUTO_TEST_CASE(set_relation)
{
  BOOST_CHECK( SetRelation::uncomparable	== SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::uncomparable	!= SetRelation::equal );
  BOOST_CHECK( SetRelation::uncomparable	!= SetRelation::properSubset );
  BOOST_CHECK( SetRelation::uncomparable	!= SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::uncomparable	!= SetRelation::disjoint );
  BOOST_CHECK( SetRelation::uncomparable	!= SetRelation::subset );
  BOOST_CHECK( SetRelation::uncomparable	!= SetRelation::superset );

  BOOST_CHECK( SetRelation::equal		!= SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::equal		== SetRelation::equal );
  BOOST_CHECK( SetRelation::equal		!= SetRelation::properSubset );
  BOOST_CHECK( SetRelation::equal		!= SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::equal		!= SetRelation::disjoint );
  BOOST_CHECK( SetRelation::equal		!= SetRelation::subset );
  BOOST_CHECK( SetRelation::equal		!= SetRelation::superset );

  BOOST_CHECK( SetRelation::properSubset	!= SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::properSubset	!= SetRelation::equal );
  BOOST_CHECK( SetRelation::properSubset	== SetRelation::properSubset );
  BOOST_CHECK( SetRelation::properSubset	!= SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::properSubset	!= SetRelation::disjoint );
  BOOST_CHECK( SetRelation::properSubset	!= SetRelation::subset );
  BOOST_CHECK( SetRelation::properSubset	!= SetRelation::superset );

  BOOST_CHECK( SetRelation::properSuperset	!= SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::properSuperset	!= SetRelation::equal );
  BOOST_CHECK( SetRelation::properSuperset	!= SetRelation::properSubset );
  BOOST_CHECK( SetRelation::properSuperset	== SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::properSuperset	!= SetRelation::disjoint );
  BOOST_CHECK( SetRelation::properSuperset	!= SetRelation::subset );
  BOOST_CHECK( SetRelation::properSuperset	!= SetRelation::superset );

  BOOST_CHECK( SetRelation::disjoint		!= SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::disjoint		!= SetRelation::equal );
  BOOST_CHECK( SetRelation::disjoint		!= SetRelation::properSubset );
  BOOST_CHECK( SetRelation::disjoint		!= SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::disjoint		== SetRelation::disjoint );
  BOOST_CHECK( SetRelation::disjoint		!= SetRelation::subset );
  BOOST_CHECK( SetRelation::disjoint		!= SetRelation::superset );

  BOOST_CHECK( SetRelation::subset		!= SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::subset		!= SetRelation::equal );
  BOOST_CHECK( SetRelation::subset		!= SetRelation::properSubset );
  BOOST_CHECK( SetRelation::subset		!= SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::subset		!= SetRelation::disjoint );
  BOOST_CHECK( SetRelation::subset		== SetRelation::subset );
  BOOST_CHECK( SetRelation::subset		!= SetRelation::superset );

  BOOST_CHECK( SetRelation::superset		!= SetRelation::uncomparable );
  BOOST_CHECK( SetRelation::superset		!= SetRelation::equal );
  BOOST_CHECK( SetRelation::superset		!= SetRelation::properSubset );
  BOOST_CHECK( SetRelation::superset		!= SetRelation::properSuperset );
  BOOST_CHECK( SetRelation::superset		!= SetRelation::disjoint );
  BOOST_CHECK( SetRelation::superset		!= SetRelation::subset );
  BOOST_CHECK( SetRelation::superset		== SetRelation::superset );
}

BOOST_AUTO_TEST_CASE(set_relation_comapre)
{
  BOOST_CHECK( SetRelation::uncomparable	== SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::uncomparable	!= SetCompare::equal );
  BOOST_CHECK( SetRelation::uncomparable	!= SetCompare::properSubset );
  BOOST_CHECK( SetRelation::uncomparable	!= SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::uncomparable	!= SetCompare::disjoint );

  BOOST_CHECK( SetRelation::equal		!= SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::equal		== SetCompare::equal );
  BOOST_CHECK( SetRelation::equal		!= SetCompare::properSubset );
  BOOST_CHECK( SetRelation::equal		!= SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::equal		!= SetCompare::disjoint );

  BOOST_CHECK( SetRelation::properSubset	!= SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::properSubset	!= SetCompare::equal );
  BOOST_CHECK( SetRelation::properSubset	== SetCompare::properSubset );
  BOOST_CHECK( SetRelation::properSubset	!= SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::properSubset	!= SetCompare::disjoint );

  BOOST_CHECK( SetRelation::properSuperset	!= SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::properSuperset	!= SetCompare::equal );
  BOOST_CHECK( SetRelation::properSuperset	!= SetCompare::properSubset );
  BOOST_CHECK( SetRelation::properSuperset	== SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::properSuperset	!= SetCompare::disjoint );

  BOOST_CHECK( SetRelation::disjoint		!= SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::disjoint		!= SetCompare::equal );
  BOOST_CHECK( SetRelation::disjoint		!= SetCompare::properSubset );
  BOOST_CHECK( SetRelation::disjoint		!= SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::disjoint		== SetCompare::disjoint );

  BOOST_CHECK( SetRelation::subset		!= SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::subset		== SetCompare::equal );
  BOOST_CHECK( SetRelation::subset		== SetCompare::properSubset );
  BOOST_CHECK( SetRelation::subset		!= SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::subset		!= SetCompare::disjoint );

  BOOST_CHECK( SetRelation::superset		!= SetCompare::uncomparable );
  BOOST_CHECK( SetRelation::superset		== SetCompare::equal );
  BOOST_CHECK( SetRelation::superset		!= SetCompare::properSubset );
  BOOST_CHECK( SetRelation::superset		== SetCompare::properSuperset );
  BOOST_CHECK( SetRelation::superset		!= SetCompare::disjoint );
}
