#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class CRingBuffer
{
public:
    explicit CRingBuffer(int iBufferLengthInSamples) :
        m_iBuffLength(iBufferLengthInSamples)
    {
        assert(iBufferLengthInSamples > 0);

        // allocate and init


        ringbuff = new T[iBufferLengthInSamples];
        
    }

    virtual ~CRingBuffer()
    {
        // free memory
        delete[] ringbuff;
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc (T tNewValue)
    {
        if (CRingBuffer::writeIdx >= CRingBuffer::m_iBuffLength) {
            CRingBuffer::writeIdx = 0;
        }
        ringbuff[CRingBuffer::writeIdx] = tNewValue;
        CRingBuffer::writeIdx++;
        CRingBuffer::lastCommandPut = true;

        return;
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue)
    {
        ringbuff[CRingBuffer::writeIdx] = tNewValue;

        return;
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc()
    {
        if (CRingBuffer::readIdx >= CRingBuffer::m_iBuffLength) {
            CRingBuffer::readIdx = 0;
        }
        T readValue = static_cast<T>(ringbuff[CRingBuffer::readIdx]);
        CRingBuffer::readIdx++;
        CRingBuffer::lastCommandPut = false;

        return readValue;
    }

    ///*! return the value at the current read index
    //\return float the value from the read index
    //*/
    //T get() const
    //{
    //    return static_cast<T>(ringbuff[CRingBuffer::readIdx]);
    //}

    /*! return the value at the current read index plus an offset
    \return float the value from the read index
    */
    T get(int iOffset = 0) const
    {
        return static_cast<T>(ringbuff[(CRingBuffer::readIdx) + iOffset]);
    }
    /*! set buffer content and indices to 0
    \return void
    */
    void reset()
    {
        for (int i = 0; i < CRingBuffer::m_iBuffLength; i++)
        {
            ringbuff[i] = 0;
        }
        CRingBuffer::readIdx = 0;
        CRingBuffer::writeIdx = 0;
        CRingBuffer::lastCommandPut = false;

        return;
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx() const
    {
        return CRingBuffer::writeIdx;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx(int iNewWriteIdx)
    {
        CRingBuffer::writeIdx = iNewWriteIdx;
        return;
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx() const
    {
        return CRingBuffer::readIdx;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx(int iNewReadIdx)
    {
        CRingBuffer::readIdx = iNewReadIdx;
        return;
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const
    {
        if (CRingBuffer::readIdx <= CRingBuffer::writeIdx) {
            return (CRingBuffer::writeIdx - CRingBuffer::readIdx);
        }
        //&&(!lastCommandPut)
        else if ((CRingBuffer::readIdx > CRingBuffer::writeIdx)) {
            return (CRingBuffer::m_iBuffLength - (CRingBuffer::readIdx - CRingBuffer::writeIdx));
        }
        else {
            return (CRingBuffer::m_iBuffLength);
        }
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const
    {
        return CRingBuffer::m_iBuffLength;
    }
private:
    CRingBuffer();
    CRingBuffer(const CRingBuffer& that);

    int m_iBuffLength;              //!< length of the internal buffer

    T* ringbuff;
    int readIdx = 0;
    int writeIdx = 0;
    bool lastCommandPut = false;
    

};
#endif // __RingBuffer_hdr__
