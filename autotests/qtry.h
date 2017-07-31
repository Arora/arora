/**
 * Copyright (c) 2008 - 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef QTRY_H
#define QTRY_H

#ifndef QTRY_COMPARE

#define __TRY_TIMEOUT__ 5000
#define __TRY_STEP__    50

#define __QTRY(__expression__, __functionToCall__) \
    do { \
        int __i = 0; \
        while (!(__expression__) &&  __i < __TRY_TIMEOUT__) { \
            QTest::qWait(__TRY_STEP__); \
            __i += __TRY_STEP__; \
        } \
        __functionToCall__; \
    } while (0)

#define QTRY_COMPARE(__expression__, __expected__) \
    __QTRY((__expression__ == __expected__), QCOMPARE(__expression__, __expected__));

#define QTRY_VERIFY(__expression__) \
    __QTRY(__expression__, QVERIFY(__expression__));

#endif // QTRY_COMPARE

#endif // QTRY_H

