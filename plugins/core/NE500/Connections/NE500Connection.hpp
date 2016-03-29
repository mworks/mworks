//
//  NE500Connection.hpp
//  NE500Plugin
//
//  Created by Christopher Stawarz on 3/29/16.
//
//

#ifndef NE500Connection_hpp
#define NE500Connection_hpp


BEGIN_NAMESPACE_MW


class NE500Connection : boost::noncopyable {
    
public:
    virtual ~NE500Connection() { }
    
    bool isConnected() const { return connected; }
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    
    virtual bool send(const std::string &command) = 0;
    virtual bool receive(std::string &response) = 0;
    
protected:
    bool connected { false };
    
};


END_NAMESPACE_MW


#endif /* NE500Connection_hpp */
