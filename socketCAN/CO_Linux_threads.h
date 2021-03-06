/**
 * Helper functions for implementing CANopen threads in Linux.
 *
 * @file        CO_Linux_threads.h
 * @ingroup     CO_socketCAN
 * @author      Janez Paternoster
 * @author      Martin Wagner
 * @copyright   2004 - 2015 Janez Paternoster
 * @copyright   2018 - 2020 Neuberger Gebaeudeautomation GmbH
 *
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CO_LINUX_THREADS_H
#define CO_LINUX_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup CO_socketCAN socketCAN
 * @{
 *
 * Linux specific interface to CANopenNode.
 *
 * CANopenNode runs on top of SocketCAN interface, which is part of the Linux
 * kernel. For more info on Linux SocketCAN see
 * https://www.kernel.org/doc/html/latest/networking/can.html
 *
 * CANopenNode runs in two threads:
 * - timer based real-time thread for CAN receive, SYNC and PDO, see
 *   CANrx_threadTmr_process()
 * - mainline thread for other processing, see threadMain_process() or
 *   threadMainWait_process()
 *
 * The "threads" specified here do not fork threads themselves, but require
 * that two threads are provided by the calling application.
 */


/**
 * Initialize mainline thread - basic.
 *
 * @param callback this function is called to indicate #threadMain_process() has
 * work to do
 * @param object this pointer is given to _callback()_
 */
void threadMain_init(void (*callback)(void*), void *object);


/**
 * Cleanup mainline thread - basic.
 */
void threadMain_close(void);


/**
 * Process mainline thread - basic.
 *
 * threadMain is non-realtime thread for CANopenNode processing. It is
 * initialized by threadMain_init(). There is no configuration for CANopen
 * objects. There is also no configuration for epool or interval timer or
 * eventfd. These must be specified externally. For more complete function see
 * threadMainWait_process(), they are included.
 *
 * threadMain_process() calls CO_process() function for processing mainline
 * CANopen objects. It is non-blocking and should be called cyclically in 50 ms
 * intervals (typically). Function must also be called immediately after
 * callback provided in threadMain_init() is called.
 *
 * @param reset return value from CO_process() function.
 */
void threadMain_process(CO_NMT_reset_cmd_t *reset);


/**
 * Initialize mainline thread - blocking.
 *
 * Function must be called always in communication reset section, after
 * CO_CANopenInit().
 */
void threadMainWait_init(void);


/**
 * Initialize once mainline thread - blocking.
 *
 * Function must be called only once, before node starts operating.
 *
 * @param interval_us interval of the threadMainWait_process()
 */
void threadMainWait_initOnce(uint32_t interval_us);


/**
 * Cleanup mainline thread - blocking.
 */
void threadMainWait_close(void);


/**
 * Process mainline thread - blocking.
 *
 * threadMainWait is non-realtime thread for CANopenNode processing. It is
 * initialized by threadMainWait_init(). There is no configuration for CANopen
 * objects. But there is configuration for epool, interval timer and eventfd.
 * Function must be called inside loop. It blocks for correct time and unblocks
 * automatically in case of event. It calls CO_process() function for processing
 * mainline CANopen objects.
 * For more basic function see threadMain_process() alternative.
 *
 * @param reset return value from CO_process() function.
 *
 * @return time difference since last call in microseconds
 */
uint32_t threadMainWait_process(CO_NMT_reset_cmd_t *reset);


/**
 * Initialize realtime thread.
 *
 * @param interval_us Interval of periodic timer in microseconds, recommended
 *                    value for realtime response: 1000 us
 */
void CANrx_threadTmr_init(uint32_t interval_us);


/**
 * Terminate realtime thread.
 */
void CANrx_threadTmr_close(void);


/**
 * Process real-time thread.
 *
 * CANrx_threadTmr is realtime thread for CANopenNode processing. It is
 * initialized by CANrx_threadTmr_init(). There is no configuration for CANopen
 * objects. But configuration for epool event notification facility is included
 * in CO_CANmodule_init() from CO_driver.c. Epool is configured to monitor the
 * following file descriptors: notify pipe, CANrx sockets from all interfaces
 * and interval timer.
 *
 * CANrx_threadTmr_process() blocks on epoll_wait(). This is implemented inside
 * CO_CANrxWait() from CO_driver.c. New CAN message is processed in
 * CANrx_threadTmr_process() function, which calls CO_process_SYNC(),
 * CO_process_RPDO() and CO_process_TPDO() functions for each expired timer
 * interval. This function must be called inside an infinite loop.
 *
 * @remark If realtime is required, this thread must be registered as such in
 * the Linux kernel.
 *
 * @return Number of interval timer passes since last call.
 */
void CANrx_threadTmr_process(void);

/** @} */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* CO_LINUX_THREADS_H */
