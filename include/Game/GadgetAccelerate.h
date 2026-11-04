#ifndef GAME_GADGETACCELERATE_H
#define GAME_GADGETACCELERATE_H

/* Advances a position by dt at a speed that changes by acceleration * dt, limited to maxSpeed
 * while it rises (header and function names inferred). Only a rising speed is limited: with a
 * negative acceleration (ElecBlock above its position) the speed falls below zero, and there is no
 * lower limit, so the descent is never capped. ElecBlock and ElecFireWall expand it inline in their
 * UpdateBody. */
static inline void Accelerate(float *position, float *speed, float acceleration, float maxSpeed,
                              float dt) {
    float startSpeed = *speed;
    float accelerationTime = dt;
    float cruiseTime = 0.0f;

    if (startSpeed >= maxSpeed) {
        accelerationTime = 0.0f;
        cruiseTime = dt;
    } else {
        float speedStep = acceleration * dt;
        float newSpeed = startSpeed + speedStep;

        if (newSpeed > maxSpeed) {
            *speed = maxSpeed;
            accelerationTime = (maxSpeed - startSpeed) / acceleration;
            cruiseTime = dt - accelerationTime;
        } else {
            *speed = newSpeed;
        }
    }

    float accelerationStep = acceleration * accelerationTime;

    *position = *position + startSpeed * accelerationTime +
                0.5f * (accelerationTime * accelerationStep) + maxSpeed * cruiseTime;
}

#endif
