//
//  AnimUtil.cpp
//
//  Created by Anthony J. Thibault on 9/2/15.
//  Copyright (c) 2015 High Fidelity, Inc. All rights reserved.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AnimUtil.h"
#include <GLMHelpers.h>
#include <NumericalConstants.h>
#include <DebugDraw.h>

// TODO: use restrict keyword
// TODO: excellent candidate for simd vectorization.

void blend(size_t numPoses, const AnimPose* a, const AnimPose* b, float alpha, AnimPose* result) {
    for (size_t i = 0; i < numPoses; i++) {
        const AnimPose& aPose = a[i];
        const AnimPose& bPose = b[i];

        // adjust signs if necessary
        const glm::quat& q1 = aPose.rot();
        glm::quat q2 = bPose.rot();
        float dot = glm::dot(q1, q2);
        if (dot < 0.0f) {
            q2 = -q2;
        }

        result[i].scale() = lerp(aPose.scale(), bPose.scale(), alpha);
        result[i].rot() = safeLerp(aPose.rot(), bPose.rot(), alpha);
        result[i].trans() = lerp(aPose.trans(), bPose.trans(), alpha);
    }
}

glm::quat averageQuats(size_t numQuats, const glm::quat* quats) {
    if (numQuats == 0) {
        return glm::quat();
    }
    glm::quat accum = quats[0];
    glm::quat firstRot = quats[0];
    for (size_t i = 1; i < numQuats; i++) {
        glm::quat rot = quats[i];
        float dot = glm::dot(firstRot, rot);
        if (dot < 0.0f) {
            rot = -rot;
        }
        accum += rot;
    }
    return glm::normalize(accum);
}

float accumulateTime(float startFrame, float endFrame, float timeScale, float currentFrame, float dt, bool loopFlag,
                     const QString& id, AnimNode::Triggers& triggersOut) {

    const float EPSILON = 0.0001f;
    float frame = currentFrame;
    const float clampedStartFrame = std::min(startFrame, endFrame);
    if (fabsf(clampedStartFrame - endFrame) <= 1.0f) {
        // An animation of a single frame should not send loop or done triggers.
        frame = endFrame;
    } else if (timeScale > EPSILON && dt > EPSILON) {
        // accumulate time, keeping track of loops and end of animation events.
        const float FRAMES_PER_SECOND = 30.0f;
        float framesRemaining = (dt * timeScale) * FRAMES_PER_SECOND;

        // prevent huge dt or timeScales values from causing many trigger events.
        uint32_t triggerCount = 0;
        const uint32_t MAX_TRIGGER_COUNT = 3;

        while (framesRemaining > EPSILON && triggerCount < MAX_TRIGGER_COUNT) {
            float framesTillEnd = endFrame - frame;
            // when looping, add one frame between start and end.
            if (loopFlag) {
                framesTillEnd += 1.0f;
            }
            if (framesRemaining >= framesTillEnd) {
                if (loopFlag) {
                    // anim loop
                    triggersOut.push_back(id + "OnLoop");
                    framesRemaining -= framesTillEnd;
                    frame = clampedStartFrame;
                } else {
                    // anim end
                    triggersOut.push_back(id + "OnDone");
                    frame = endFrame;
                    framesRemaining = 0.0f;
                }
                triggerCount++;
            } else {
                frame += framesRemaining;
                framesRemaining = 0.0f;
            }
        }
    }
    return frame;
}

// rotate bone's y-axis with target.
AnimPose boneLookAt(const glm::vec3& target, const AnimPose& bone) {
    glm::vec3 u, v, w;
    generateBasisVectors(target - bone.trans(), bone.rot() * Vectors::UNIT_X, u, v, w);
    glm::mat4 lookAt(glm::vec4(v, 0.0f),
                     glm::vec4(u, 0.0f),
                     // AJT: TODO REVISIT THIS, this could be -w.
                     glm::vec4(glm::normalize(glm::cross(v, u)), 0.0f),
                     glm::vec4(bone.trans(), 1.0f));
    return AnimPose(lookAt);
}

// This will attempt to determine the proper body facing of a characters body
// assumes headRot is z-forward and y-up.
// and returns a bodyRot that is also z-forward and y-up
glm::quat computeBodyFacingFromHead(const glm::quat& headRot, const glm::vec3& up) {

    glm::vec3 bodyUp = glm::normalize(up);

    // initially take the body facing from the head.
    glm::vec3 headUp = headRot * Vectors::UNIT_Y;
    glm::vec3 headForward = headRot * Vectors::UNIT_Z;
    glm::vec3 headLeft = headRot * Vectors::UNIT_X;
    const float NOD_THRESHOLD = cosf(glm::radians(45.0f));
    const float TILT_THRESHOLD = cosf(glm::radians(30.0f));

    glm::vec3 bodyForward = headForward;

    float nodDot = glm::dot(headForward, bodyUp);
    float tiltDot = glm::dot(headLeft, bodyUp);

    if (fabsf(tiltDot) < TILT_THRESHOLD) { // if we are not tilting too much
        if (nodDot < -NOD_THRESHOLD) { // head is looking downward
            // the body should face in the same direction as the top the head.
            bodyForward = headUp;
        } else if (nodDot > NOD_THRESHOLD) {  // head is looking upward
            // the body should face away from the top of the head.
            bodyForward = -headUp;
        }
    }

    // cancel out upward component
    bodyForward = glm::normalize(bodyForward - nodDot * bodyUp);

    glm::vec3 u, v, w;
    generateBasisVectors(bodyForward, bodyUp, u, v, w);

    // create matrix from orthogonal basis vectors
    glm::mat4 bodyMat(glm::vec4(w, 0.0f), glm::vec4(v, 0.0f), glm::vec4(u, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    return glmExtractRotation(bodyMat);
}
