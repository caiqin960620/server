#ifndef SINGLETON
#define SINGLETON

class CNoncopyable
{
protected:
    CNoncopyable() {}
    ~CNoncopyable() {}
private:  
    CNoncopyable( const CNoncopyable& );
    const CNoncopyable& operator=( const CNoncopyable& );
};

template <class ClassType>
class CSingleton : public CNoncopyable
{
public:
    static ClassType& Inst()
    {
        static ClassType s_instance;
        return s_instance;
    }
};

#endif