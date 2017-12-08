/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

//LUM_WEIGHTS is used to calculate the intensity of a color value
// (note green contributes more than red contributes more than blue)
#define LUM_WEIGHTS float3(0.27f, 0.67f, 0.06f)
//MIDDLE_GREY is used to scale the color values for tone mapping
#define MIDDLE_GREY 0.08f
//BLOOM_THRESHOLD is the lower limit for intensity values that will contribute to bloom
#define BLOOM_THRESHOLD 0.5f
//BLOOM_MULTIPLIER scales the bloom value prior to adding the bloom value to the material color
#define BLOOM_MULTIPLIER	0.6f

#define GAMMA_POW 2.2
#define DELTA 0.00000001f

#define BLURH_THREADS_X 4
#define BLURH_THREADS_Y 64

#define BLURV_THREADS_X 64
#define	BLURV_THREADS_Y 4

#define ADD_THREADS_X 64
#define ADD_THREADS_Y 2

#define LUM_AND_BRIGHT_THREADS 8u

#define REDUCE_GROUP_SIZE 64

#define GET_AVG_LUM_THREADS 16u
#define REDUCE_Y_PER_THREAD 8
