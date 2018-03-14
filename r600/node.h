#ifndef NODE_H
#define NODE_H

#include <cstdint>
#include <iosfwd>
#include <vector>

class node
{
public:
        node();
        node(unsigned bytecode_size);
        int bytecode_size() const;

        friend std::ostream& operator << (std::ostream&os, const node& n);

        void append_bytecode(std::vector<uint64_t>& program) const;
protected:
        static const char *component_names;
private:
        int m_bytecode_size;
        virtual void do_append_bytecode(std::vector<uint64_t>& program) const = 0;
        virtual void print(std::ostream& os) const = 0;
};

inline std::ostream& operator << (std::ostream&os, const node& n)
{
        n.print(os);
        return os;
}

#endif // NODE_H