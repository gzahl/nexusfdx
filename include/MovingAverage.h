 /* Copyright (c) 2014 LAAS-CNRS
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
 
#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H
 
template <class T>
/** class of moving average
* Example:
 * @code
 * #include "mbed.h"
 * #include "MovingAverage.h"
 * #define NSAMPLE 100
 * Ticker flipperADC;
 * AnalogIn   ain(A0);
 * MovingAverage <float>vavg(NSAMPLE,0.0);
 * void flipADC()
 * {
 *     vavg.Insert(ain.read());
 * }
 * int main()
 * { 
 *   flipperADC.attach_us(&flipADC, 10000);
 *   while (true) 
 *      printf("analog= %f \r\n",vavg.GetAverage()); 
 * }
 * @endcode
 */
 
class MovingAverage
{
private:
    T* Element;
    T Average;
    
    unsigned char NextElement;
    unsigned char MaxLength;
public:
    /** Create  moving average
     * @param maxLength is length of moving average
     * @param value is initial value
     */
    MovingAverage(unsigned char maxLength, T defaultValue);
    /** Get the value of the moving average
     * @return value of the moving average
     */
    T GetAverage();
    /** Insert a element in moving average
     * @param value is the element inserted in moving average
     */
 
    void Insert(T value);
};
 
template<class T>
MovingAverage<T>::MovingAverage(unsigned char maxLength, T defaultValue){
    MaxLength = maxLength;
    
    Element = new T[MaxLength];
    
    for(int i = 0; i<MaxLength;i++)
    {
        Element[i] = defaultValue;
    }
    Average = defaultValue;
}
 
template<class T>
T MovingAverage<T>::GetAverage(){
    return Average;
}
 
 
template<class T>
void MovingAverage<T>::Insert(T value){
    Average = value/MaxLength + Average - Element[++NextElement]/MaxLength;
    Element[NextElement] = value;
    if(NextElement>=(MaxLength-1))   
        NextElement=0;
    
}
#endif //MOVING_AVERAGE_H