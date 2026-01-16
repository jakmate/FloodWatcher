// tests/cpp/unit/StationClusterTest.cpp
#include "StationCluster.hpp"
#include <gtest/gtest.h>

class QuadTreeTest : public ::testing::Test {
  protected:
    Bounds ukBounds{49.0, 61.0, -8.0, 2.0};
};

TEST_F(QuadTreeTest, InsertSinglePoint) {
  QuadTreeNode tree(ukBounds, 10);
  ClusterPoint point{51.5, -0.1, 0};

  tree.insert(point);
  auto clusters = tree.getClusters(10.0, 0.01);

  ASSERT_EQ(clusters.size(), 1);
  EXPECT_DOUBLE_EQ(clusters[0].lat, 51.5);
  EXPECT_DOUBLE_EQ(clusters[0].lon, -0.1);
  EXPECT_EQ(clusters[0].count, 1);
}

TEST_F(QuadTreeTest, InsertOutOfBoundsIgnored) {
  QuadTreeNode tree(ukBounds, 10);
  ClusterPoint point{70.0, 10.0, 0}; // Outside UK bounds

  tree.insert(point);
  auto clusters = tree.getClusters(10.0, 0.01);

  EXPECT_EQ(clusters.size(), 0);
}

TEST_F(QuadTreeTest, SubdivisionTriggersAfter10Points) {
  QuadTreeNode tree(ukBounds, 10);

  // Insert 11 points to trigger subdivision
  for (int i = 0; i < 11; ++i) {
    ClusterPoint point{50.0 + i * 0.1, -1.0, i};
    tree.insert(point);
  }

  auto clusters = tree.getClusters(15.0, 0.001); // High zoom to see all points
  EXPECT_EQ(clusters.size(), 11);
}

TEST_F(QuadTreeTest, ClusteringAtLowZoom) {
  QuadTreeNode tree(ukBounds, 10);

  // Insert 20 nearby points
  for (int i = 0; i < 20; ++i) {
    ClusterPoint point{51.5 + i * 0.01, -0.1, i};
    tree.insert(point);
  }

  auto clusters = tree.getClusters(6.0, 0.1); // Low zoom

  // Should cluster into fewer groups
  EXPECT_LT(clusters.size(), 20);
  EXPECT_GT(clusters.size(), 0);
}

TEST_F(QuadTreeTest, IndividualPointsAtHighZoom) {
  QuadTreeNode tree(ukBounds, 10);

  for (int i = 0; i < 5; ++i) {
    ClusterPoint point{51.5 + i * 0.01, -0.1, i};
    tree.insert(point);
  }

  auto clusters = tree.getClusters(14.0, 0.0005); // High zoom

  // Should show individual points
  EXPECT_EQ(clusters.size(), 5);
  for (const auto& cluster : clusters) {
    EXPECT_EQ(cluster.count, 1);
  }
}

TEST_F(QuadTreeTest, StationIndicesPreserved) {
  QuadTreeNode tree(ukBounds, 10);

  ClusterPoint point1{51.5, -0.1, 42};
  ClusterPoint point2{51.6, -0.2, 99};

  tree.insert(point1);
  tree.insert(point2);

  auto clusters = tree.getClusters(14.0, 0.001);

  ASSERT_EQ(clusters.size(), 2);
  EXPECT_EQ(clusters[0].stationIndices[0], 42);
  EXPECT_EQ(clusters[1].stationIndices[0], 99);
}

TEST_F(QuadTreeTest, ClusterAggregatesCorrectly) {
  QuadTreeNode tree(ukBounds, 10);

  // Three points that should cluster
  tree.insert({51.0, -1.0, 0});
  tree.insert({51.01, -1.01, 1});
  tree.insert({51.02, -1.02, 2});

  auto clusters = tree.getClusters(8.0, 0.05);

  // Should form one cluster
  ASSERT_EQ(clusters.size(), 1);
  EXPECT_EQ(clusters[0].count, 3);

  // Check centroid is average
  double expectedLat = (51.0 + 51.01 + 51.02) / 3.0;
  double expectedLon = (-1.0 - 1.01 - 1.02) / 3.0;
  EXPECT_NEAR(clusters[0].lat, expectedLat, 0.001);
  EXPECT_NEAR(clusters[0].lon, expectedLon, 0.001);
}

TEST_F(QuadTreeTest, MaxDepthLimitsSubdivision) {
  QuadTreeNode tree(ukBounds, 2); // Max depth 2

  // Insert many points to try to force subdivision
  for (int i = 0; i < 100; ++i) {
    ClusterPoint point{50.0 + i * 0.001, -1.0, i};
    tree.insert(point);
  }

  // Should still return results (won't crash from over-subdivision)
  auto clusters = tree.getClusters(10.0, 0.01);
  EXPECT_GT(clusters.size(), 0);
}

TEST_F(QuadTreeTest, EmptyTreeReturnsNoClusters) {
  QuadTreeNode tree(ukBounds, 10);

  auto clusters = tree.getClusters(10.0, 0.01);

  EXPECT_EQ(clusters.size(), 0);
}
