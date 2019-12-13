/*  ------------------------------------------------------------------
    Copyright (c) 2019 Marc Toussaint
    email: marc.toussaint@informatik.uni-stuttgart.de

    This code is distributed under the MIT License.
    Please see <root-path>/LICENSE for details.
    --------------------------------------------------------------  */

#include "objective.h"
#include <Kin/switch.h>
#include <Core/graph.h>

//===========================================================================

void Objective::setCostSpecs(int fromStep, int toStep, bool sparse) {
  CHECK_GE(fromStep, 0, "");
//  CHECK_GE(toStep, fromStep, "");
  if(!sparse) {
    if(toStep>=fromStep)
      vars.resize(toStep+1).setZero();
    else vars.clear();
    for(int t=fromStep; t<=toStep; t++) vars(t) = 1;
  } else {
    if(toStep>=fromStep)
      vars.resize(1+toStep-fromStep, map->order+1);
    else vars.resize(0, map->order+1);
    for(int t=fromStep; t<=toStep; t++)
      for(uint j=0; j<vars.d1; j++) vars(t-fromStep, j) = t+j-int(map->order);
  }
}

void Objective::setCostSpecs(const arr& times, int stepsPerPhase, uint T,
                             int deltaFromStep, int deltaToStep, bool sparse) {

  double fromTime=0, toTime=-1.;
  if(!times.N) {
  } else if(times.N==1) {
    fromTime = toTime = times(0);
  } else {
    CHECK_EQ(times.N, 2, "");
    fromTime = times(0);
    toTime = times(1);
  }

  if(toTime>double(T)/stepsPerPhase+1.) {
    LOG(-1) <<"beyond the time!: endTime=" <<toTime <<" phases=" <<double(T)/stepsPerPhase;
  }

  CHECK_GE(stepsPerPhase, 0, "");

  int fromStep = (fromTime<0.?0:conv_time2step(fromTime, stepsPerPhase));
  int toStep   = (toTime<0.?T-1:conv_time2step(toTime, stepsPerPhase));

  if(fromTime>=0 && deltaFromStep) fromStep+=deltaFromStep;
  if(toTime>=0 && deltaToStep) toStep+=deltaToStep;

  if(fromStep<0) fromStep=0;
//  if(toStep<0) toStep=0;
  if(toStep>=(int)T && T>0) toStep=T-1;

  setCostSpecs(fromStep, toStep, sparse);
}

bool Objective::isActive(uint t) {
  if(!vars.N) return false;
  CHECK_EQ(vars.nd, 1, "variables are not time indexed (tuples for dense problem instead)");
  return (vars.N>t && vars(t));
}

void Objective::write(std::ostream& os) const {
  os <<"TASK '" <<name <<"'";
  if(vars.N) {
    if(vars.nd==1) {
      if(vars.N>4) writeConsecutiveConstant(os, vars);
      else os <<" ("<<vars <<')';
    } else os <<" (" <<vars.first() <<".." <<vars.last() <<')';
  } else os <<" ()";
  os <<"  type:" <<type
     <<"  order:" <<map->order
     <<"  target:" <<map->target
     <<"  scale:" <<map->scale;
}

//===========================================================================
