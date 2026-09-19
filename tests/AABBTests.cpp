#include "doctest/doctest.h"

#include "core/AABB.h"

TEST_SUITE("AABB")
{
	TEST_CASE("Overlaps is true for two boxes that intersect")
	{
		const AABB a{ 0.0f, 0.0f, 10.0f, 10.0f };
		const AABB b{ 5.0f, 5.0f, 15.0f, 15.0f };

		CHECK(a.Overlaps(b));
		CHECK(b.Overlaps(a));
	}

	TEST_CASE("Overlaps is false for two boxes that only touch at an edge")
	{
		const AABB a{ 0.0f, 0.0f, 10.0f, 10.0f };
		const AABB b{ 10.0f, 0.0f, 20.0f, 10.0f };

		CHECK_FALSE(a.Overlaps(b));
	}

	TEST_CASE("Overlaps is false for two boxes that are far apart")
	{
		const AABB a{ 0.0f, 0.0f, 10.0f, 10.0f };
		const AABB b{ 100.0f, 100.0f, 110.0f, 110.0f };

		CHECK_FALSE(a.Overlaps(b));
	}

	TEST_CASE("FeetAABB anchors the box at the horizontal centre and bottom edge")
	{
		const AABB box = FeetAABB(100.0f, 50.0f, 20.0f, 30.0f);

		CHECK(box.left == doctest::Approx(90.0f));
		CHECK(box.right == doctest::Approx(110.0f));
		CHECK(box.top == doctest::Approx(20.0f));
		CHECK(box.bottom == doctest::Approx(50.0f));
	}
}
