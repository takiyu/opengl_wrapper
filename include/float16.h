#ifndef FLOAT16_H_190330
#define FLOAT16_H_190330

namespace oglw {

// ------------------------------ Dummy Half float -----------------------------
// Just wrap float32 for CPU
class Float16 {
public:
    Float16() {}
    Float16(float v_) : v(v_) {}
    Float16 operator += (Float16 lhs) { v += lhs.v; return *this; }
    Float16 operator -= (Float16 lhs) { v -= lhs.v; return *this; }
    Float16 operator *= (Float16 lhs) { v *= lhs.v; return *this; }
    Float16 operator /= (Float16 lhs) { v /= lhs.v; return *this; }
    Float16 operator + (Float16 lhs) const { return Float16(v + lhs.v); }
    Float16 operator - (Float16 lhs) const { return Float16(v - lhs.v); }
    Float16 operator * (Float16 lhs) const { return Float16(v * lhs.v); }
    Float16 operator / (Float16 lhs) const { return Float16(v / lhs.v); }
    Float16 operator - () const { return Float16(-v); }
    bool operator == (Float16 lhs) const { return v == lhs.v; }
    bool operator != (Float16 lhs) const { return v != lhs.v; }

private:
    float v;
};

}  // namespace oglw

#endif /* end of include guard */
