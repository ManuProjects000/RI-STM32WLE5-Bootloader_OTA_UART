/**
 * @file r_flags.h
 * @brief RTOS task flag definitions.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 * @details
 * This file defines bit flags used by tasks in the RTOS to signal events.
 * Each flag corresponds to a specific event or condition.
 */
 
 
// RTOS TASK FLAGS
#ifndef R_FLAGS_H
#define R_FLAGS_H

/** Transmission event flag */
#define FLAG_TX  (1 << 0)

/** Flag indicating an OTA update has been received */
#define FLAG_RECEIVE_UPDATE	(1 << 1)

#endif // R_FLAGS_H