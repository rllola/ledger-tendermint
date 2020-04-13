/*******************************************************************************
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "buffering.h"
#include "vote_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RAM_BUFFER_SIZE 1024

#if defined(TARGET_NANOX)
#undef RAM_BUFFER_SIZE
#define RAM_BUFFER_SIZE 16384
#endif

uint8_t ram_buffer[RAM_BUFFER_SIZE];

void vote_initialize() {
    buffering_init(ram_buffer, sizeof(ram_buffer), NULL, 0);
}

uint32_t vote_append(unsigned char *buffer, uint32_t length) {
    return buffering_append(buffer, length);
}

uint32_t vote_get_buffer_length() {
    return buffering_get_buffer()->pos;
}

const uint8_t *vote_get_buffer() {
    return buffering_get_buffer()->data;
}

#ifdef __cplusplus
}
#endif
