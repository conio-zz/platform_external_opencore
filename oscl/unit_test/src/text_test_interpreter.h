/* ------------------------------------------------------------------
 * Copyright (C) 2008 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef TEXT_TEST_INTERPRETER_H
#define TEXT_TEST_INTERPRETER_H

#ifndef TEST_RESULT_H
#include "test_result.h"
#endif


//a simple interpreter which outputs a result of testing
class text_test_interpreter
{
    protected:
        //partial interpretations
        _STRING header(const test_result& result) const;
        _STRING footer(const test_result& result) const;
        _STRING successes(const test_result& result) const;
        _STRING failures(const test_result& result) const;
        _STRING errors(const test_result& result) const;
        _STRING problem_vector_string(const _VECTOR(test_problem, unit_test_allocator)&
                                      vect) const;
        _STRING problem_string(const test_problem& problem) const;
    public:
        //returns the interpretation of a test result
        _STRING interpretation(const test_result& result_to_interpret) const;
};

#endif
