//
// Created by Arseny Tolmachev on 2017/02/26.
//

#ifndef JUMANPP_LATTICE_TYPES_H
#define JUMANPP_LATTICE_TYPES_H

#include <core/core_types.h>
#include "util/soa.h"
#include "util/types.hpp"

namespace jumanpp {
namespace core {
namespace analysis {

using LatticePosition = u16;
using Score = float;

struct ConnectionPtr {
  u16 boundary;
  u16 left;
  u16 right;
  u16 beam;
};

struct LatticeNodePtr {
  u16 boundary;
  u16 position;
};

struct LatticeConfig {
  u32 numBoundaries;
  u32 numPrimitiveFeatures;
  u32 numFeaturePatterns;
  u32 numFinalFeatures;
  u32 beamSize;
};

struct LatticeBoundaryConfig {
  u32 boundary;
  u32 endNodes;
  u32 beginNodes;
};

struct ConnectionBeamElement {
  ConnectionPtr ptr;
  Score totalScore;
};

class LatticeLeftBoundary final : public util::memory::StructOfArrays {
  util::memory::SizedArrayField<LatticeNodePtr> endingNodes;

 public:
  LatticeLeftBoundary(util::memory::ManagedAllocatorCore* alloc,
                      const LatticeConfig& lc,
                      const LatticeBoundaryConfig& lbc);
};

class LatticeRightBoundary final : public util::memory::StructOfArrays {
  util::memory::SizedArrayField<i32> dicPtrs;
  util::memory::SizedArrayField<i32, 64> primitiveFeatures;
  util::memory::SizedArrayField<u64, 64> featurePatterns;
  util::memory::SizedArrayField<ConnectionBeamElement, 64> beam;

  friend class LatticeBoundary;

 public:
  LatticeRightBoundary(util::memory::ManagedAllocatorCore* alloc,
                       const LatticeConfig& lc,
                       const LatticeBoundaryConfig& lbc);
};

class LatticeBoundaryConnection final
    : public util::memory::StructOfArraysFactory<LatticeBoundaryConnection> {
  util::memory::SizedArrayField<u32, 64> features;
  util::memory::SizedArrayField<Score> featureScores;

 public:
  LatticeBoundaryConnection(util::memory::ManagedAllocatorCore* alloc,
                            const LatticeConfig& lc,
                            const LatticeBoundaryConfig& lbc);

  LatticeBoundaryConnection(const LatticeBoundaryConnection& o);
};

class LatticeBoundary {
  LatticeBoundaryConfig config;
  LatticeLeftBoundary left;
  LatticeRightBoundary right;
  LatticeBoundaryConnection connections;

  Status initialize() {
    JPP_RETURN_IF_ERROR(left.initialize());
    JPP_RETURN_IF_ERROR(right.initialize());
    JPP_RETURN_IF_ERROR(connections.initialize());
    return Status::Ok();
  }

 public:
  LatticeBoundary(util::memory::ManagedAllocatorCore* alloc,
                  const LatticeConfig& lc, const LatticeBoundaryConfig& lbc);

  EntryPtr entry(u32 position) const {
    return EntryPtr{right.dicPtrs.data().at(position)};
  }

  friend class Lattice;
};

class Lattice {
  util::memory::ManagedVector<LatticeBoundary> boundaries;
  LatticeConfig lconf;
  util::memory::ManagedAllocatorCore* alloc;

 public:
  Lattice(util::memory::ManagedAllocatorCore* alloc, const LatticeConfig& lc);
  u32 createdBoundaryCount() const { return (u32)boundaries.size(); }
  Status makeBoundary(const LatticeBoundaryConfig& lbc, LatticeBoundary** ptr);

  LatticeBoundary* boundary(u32 idx) { return &boundaries.at(idx); }
  const LatticeBoundary* boundary(u32 idx) const { return &boundaries.at(idx); }
};

}  // analysis
}  // core
}  // jumanpp

#endif  // JUMANPP_LATTICE_TYPES_H