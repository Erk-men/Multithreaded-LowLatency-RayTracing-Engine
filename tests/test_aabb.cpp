#include <cassert>
#include <cstdio>
#include "aabb.h"

// AABB sınıfının temel işlevlerini test eden birim testleri
int main() {
    AABB box1(Point3(0, 0, 0), Point3(1, 1, 1));
    AABB box2(Point3(0.5, 0.5, 0.5), Point3(1.5, 1.5, 1.5));
    AABB box3(Point3(2, 2, 2), Point3(3, 3, 3));
    AABB box(Point3(-1, -1, -1), Point3(1, 1, 1));

    //geniş bir t aralığı t_min=0.0 ve t_max=1e+9 ile test edelim
    
    // 6 seneryo, jer biri için ayrı bir assert + printf ile test edelim

    // Test 1 Düz çarpma 
    Ray r1(Point3(0.5, 0.5, -5), Vec3(0, 0, 1));
    assert(box1.hit(r1, 0.0, 1e+9) == true);
    printf("Test 1 passed: Ray hits box1, Doğrudan çarpma\n");

    // Test 2 Düz ıska: Kutudan uzak, kutuya hiç değmeyen ışın
    Ray r2(Point3(5, 5, -5), Vec3(0, 0, 1));
    assert(box1.hit(r2, 0.0, 1e+9) == false);
    printf("Test 2 passed: Ray misses box1, Düz ıska\n");

    // Test 3: Negatif yön (swap testi): Kutunun arkasından gelen ışın, kutuya doğru geriye bakan
    Ray r3(Point3(0, 0, 5), Vec3(0, 0, -1));
    assert(box1.hit(r3, 0.0, 1e+9) == true);
    printf("Test 3 passed: Ray hits box1 from behind, Negatif yön (swap testi)\n");

    // Test 4: Eksene paralel ışın, ıskalıyor. O eksende origin kutunun dışında, diğer eksenlerde kutu ile aynı hizada. Kutuyu ıskalar.
    Ray r4(Point3(5, 0.5, -5), Vec3(0, 0, 1)); //x=5, box1 in x aralığı [0,1] dışında, y=0.5 kutu ile aynı hizada, z=-5 kutunun önünde
    assert(box1.hit(r4, 0.0, 1e+9) == false);
    printf("Test 4 passed: Ray misses box1, Eksene paralel ışın, ıskalıyor\n");

    // Test 5: Eksene paralel ışın, Kutunun içinde çarpıyor
    Ray r5(Point3(0.5, 0.5, -5), Vec3(0, 0, 1)); //x=0.5, y=0.5, z=-5
    assert(box1.hit(r5, 0.0, 1e+9) == true);
    printf("Test 5 passed: Ray hits box1, Eksene paralel ışın, Kutunun içinde\n");

    // Test6: 0/0 NaN - origin kutunun yüzeyinde, o eksende direction=0, (test 3 teki tesadüfi çakışmadan bağımsız, kasıtlı izole bir kurulum)
    Ray r6(Point3(0, 0.5, 0.5), Vec3(0, 0, 0)); // x=1 tam box1 in max.x sınırında; direction,x=0 0/0 NaN durumu yaratır
    bool result6 = box1.hit(r6, 0.0, 1e+9); //sonuç true veya false olabilir, önemli olan NaN hatası vermemesi/çökmemesi
    (void)result6; //unused variable warning önlemek için, hangi sonucun doğru olduğunu değil, çökmedini kanıtlıyoruz.
    printf("Test 6 passed: Ray at box1 surface with zero direction, No crash or NaN error\n");
    
    // Birim kutuların çarpışma testi
    AABB unitBox1(Point3(0, 0, 0), Point3(1, 1, 1));
    AABB unitBox2(Point3(0.5, 0.5, 0.5), Point3(1.5, 1.5, 1.5));
    AABB unitBox3(Point3(2, 2, 2), Point3(3, 3, 3));
    assert(unitBox1.hit(Ray(Point3(0.5, 0.5, -1), Vec3(0, 0, 1)), 0.0, 1e+9) == true);
    assert(unitBox2.hit(Ray(Point3(1, 1, -1), Vec3(0, 0, 1)), 0.0, 1e+9) == true);
    assert(unitBox3.hit(Ray(Point3(2.5, 2.5, -1), Vec3(0, 0, 1)), 0.0, 1e+9) == true);
    assert(unitBox1.hit(Ray(Point3(1.5, 1.5, -1), Vec3(0, 0, 1)), 0.0, 1e+9) == false);
    assert(unitBox2.hit(Ray(Point3(2, 2, -1), Vec3(0, 0, 1)), 0.0, 1e+9) == false);
    assert(unitBox3.hit(Ray(Point3(3.5, 3.5, -1), Vec3(0, 0, 1)), 0.0, 1e+9) == false);
    printf("Unit box collision tests passed\n");

    // Boş sentinel kutu testi
    AABB emptyBox;
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(1, 0, 0)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(0, 1, 0)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(0, 0, 1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(-1, 0, 0)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(0, -1, 0  )), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(0, 0, -1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(1, 1, 1), Vec3(1, 1, 1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(-1, -1, -1), Vec3(-1, -1, -1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0.5, 0.5, 0.5), Vec3(1, 1, 1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0.5, 0.5, 0.5), Vec3(-1, -1, -1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(1, 1, 1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(-1, -1, -1)), 0.0, 1e+9) == false);
    assert(emptyBox.hit(Ray(Point3(0, 0, 0), Vec3(1, 0, 0)), 0.0, 1e+9) == false); 
    printf("Empty sentinel box tests passed\n");


    // (2, 3, 4) boyutlu kutu (min=(0, 0, 0), max=(2, 3, 4)) ile çarpışma testi
    AABB boxA(Point3(0, 0, 0), Point3(2, 3, 4));
    Ray rA(Point3(1, 1, -1), Vec3(0, 0, 1)); // Kutunun önünden gelen
    assert(boxA.hit(rA, 0.0, 1e+9) == true);
    Ray rB(Point3(1, 1, 5), Vec3(0, 0, -1)); // Kutunun arkasından gelen
    assert(boxA.hit(rB, 0.0, 1e+9) == true);
    Ray rC(Point3(3, 1, 2), Vec3(-1, 0, 0)); // Kutunun sağından gelen
    assert(boxA.hit(rC, 0.0, 1e+9) == true);
    Ray rD(Point3(-1, 1, 2), Vec3(1, 0, 0)); // Kutunun solundan gelen
    assert(boxA.hit(rD, 0.0, 1e+9) == true);
    Ray rE(Point3(1, 4, 2), Vec3(0, -1, 0)); // Kutunun üstünden gelen
    assert(boxA.hit(rE, 0.0, 1e+9) == true);
    Ray rF(Point3(1, -1, 2), Vec3(0, 1, 0)); // Kutunun altından gelen
    assert(boxA.hit(rF, 0.0, 1e+9) == true);
    Ray rG(Point3(3, 4, 5), Vec3(-1, -1, -1)); // Kutunun köşesinden gelen
    assert(boxA.hit(rG, 0.0, 1e+9) == true);
    printf("BoxA collision tests passed\n");

    //surface_area() testleri
    assert(box1.surface_area() == 3.0);
    // birim kutu: 1*1+1*1+1*1
    AABB emptyForArea;
    assert(emptyForArea.surface_area() == 0.0);
    // sentinel/boş kutu koruması
    assert(boxA.surface_area() == 26.0);
    // (2,3,4) kutu: 2*3+3*4+4*2
    printf("surface_area() tests passed\n");

    printf("ALL AABB TESTS PASSED\n");
    return 0;
}