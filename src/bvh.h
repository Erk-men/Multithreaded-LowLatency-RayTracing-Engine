#pragma once
#include "hittable.h"
#include "aabb.h"
#include <vector>
#include <algorithm> // std::nth_element için
#include <chrono> // zaman ölçümü için

// Yaprak eşiği: Bir düğümdeki nesne sayısı bu değere düşünce dallanmayı durdur, yaprak yap
// constexpr - CLI bayrağı değil, TILE_SIZE/MAX_THREADS deseniyle aynı gerekçe:
// Deneyle ayarlanabilir ama kullanıcıya sorulmaya değmeyen bir iç uygulama detayyı

constexpr int BVH_LEAF_THRESHOLD = 4; 
// Node Struct
struct BVHNode {
    AABB box; // Bu düğümün AABB si
    int left_first; // Internal düğüm: Sol çocuğun index i (sağ = left_first+1) | Yaprak indices dizisindeki ilk nesnenin index i
    int prim_count; // Internal düğüm: 0 | >0 = yaprak, prim_count kadar nesne içerir
    int split_axis; // Hangi eksende bölündüğünü belirtir: 0 = x, 1 = y, 2 = z, near-first traversal için kullanılır.

    bool is_leaf() const { return prim_count > 0; } // Yaprak mı kontrolü

};

class Bvh : public Hittable {
    std::vector<Hittable*> objects; // Kopyalanan pointer listesi(D-04: Pointee 'ler taşınmaz, sadece pointer kopyalanır)
    std::vector<int> indices; // Primitive index array, bu yeniden sıralanabilir objects değil ve pointer'lar taşınmaz, sadece index'ler taşınır
    std::vector<BVHNode> nodes; // BVH düğümleri, heap üzerinde saklanır ve pointer'lar taşınmaz(flat arena)
    int nodes_used = 1; // nodes[0] root düğümüdür, 1 den başlar çünkü root zaten kullanılmıştır.
    int tree_depth = 0; // Ağacın derinliği, istatistiksel amaçlar için tutulur
    long build_ms = 0; // Ağacın inşa süresi, istatistiksel amaçlar için tutulur

    Point3 centroid_of(int prim_idx) const{
        AABB box = objects[prim_idx]->bounding_box();
        return (box.min + box.max) * 0.5; // AABB'nin merkezini döndür
    }
    void subdivide(int node_idx, int first, int count, int depth); // Düğümü bölmek için yardımcı fonksiyon

public:
    // Constructor, BVH'yi oluşturur
    explicit Bvh(const std::vector<Hittable*>& objs) { 
        objects = objs; // pointer kopyası

        if (objects.empty()) {
            nodes.resize(1); // Boş sahne için tek bir sentinel düğüm
            nodes[0].prim_count = 0; // Ya da 0 nesneli bir yaprak olarak işaretlenebilir
            return;
        }

        int n = (int)objects.size();
        indices.resize(n);
        for (int i = 0; i < n; ++i) indices[i] = i; // indices array'ini başlat
        nodes.resize(2 * n-1); // Idiom A: landmine ı yapısal olarak imkansız kılıyor, çünkü her yaprak bir nesne içerir ve her internal düğüm en az 2 çocuğa sahiptir. Bu nedenle, n nesne için en fazla 2n-1 düğüm gerekir.
        auto start = std::chrono::high_resolution_clock::now();
        subdivide(0, 0, n, 0); // root düğüm, index 0, ilk nesne 0, count = n, depth = 0
        auto end = std::chrono::high_resolution_clock::now();
        build_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        nodes.resize(nodes_used); // nodes_used kadar düğüm kullanıldı, fazlalıkları kes
    }

    // Dışarıdan erişim için getter fonksiyonları
    int stat_nodes() const { return (int)nodes.size(); }
    int stat_depth() const { return tree_depth; }
    long stat_build_ms() const { return build_ms; }

    double stat_avg_leaf() const {
        long sum = 0, leaf_count = 0;
        for (const auto& node : nodes) {
            if (node.is_leaf()) {
                sum += node.prim_count;
                ++leaf_count;
            }
        }
        return leaf_count > 0 ? (double)sum / leaf_count : 0.0;
    }

    std::vector<int> leaf_size_histogram() const {
        std::vector<int> hist;
        for (const auto& node : nodes) {
            if (node.is_leaf()) {
                if ((int)hist.size() <= node.prim_count) 
                hist.resize(node.prim_count + 1);
                hist[node.prim_count]++;
            }
        }
        return hist;
    }
    //hit(), bounding_box(), stat accessorları ve diğer yardımcı fonksiyonlar burada tanımlanabilir 

    // Hittable interface override
    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override;
        AABB bounding_box() const override {
            return nodes.empty() ? AABB() : nodes[0].box; // root düğümün kutusunu döndür
    }
};


void Bvh::subdivide(int node_idx, int first, int count, int depth) {
    // 1-Düğümün kutusunu, aralıktaki tüm nesnelerin bounding_box larının birleşimi yap.
    AABB box;
    for (int i = first; i < first + count; ++i) 
        box.grow(objects[indices[i]]->bounding_box());
    nodes[node_idx].box = box; // Düğümün kutusunu ayarla
    
    tree_depth = std::max(tree_depth, depth); // Ağacın derinliğini güncelle

    // 2- Yaprak mı? - Eşik altındaysa dur.
    if (count <= BVH_LEAF_THRESHOLD) {
        nodes[node_idx].left_first = first; // Yaprak: indices dizisindeki ilk nesne
        nodes[node_idx].prim_count = count; // Yaprak: nesne sayısı
        return;
    }

    // 3- Centroid kutusunu kur, en geniş yayılan ekseni seç.
    AABB centroid_box;
    for (int i = first; i < first + count; ++i)
        centroid_box.grow(centroid_of(indices[i]));
    int axis = centroid_box.longest_axis(); // 0=x, 1=y, 2=z

    // 4- Ortancaya göre böl: std::nth_element kullanarak, pivot olarak ortanca centroid seç.
    int mid = first + count / 2;
    std::nth_element(indices.begin() + first, indices.begin() + mid, indices.begin() + first + count, [this, axis](int a, int b) {
        return centroid_of(a)[axis] < centroid_of(b)[axis];
    });

    // 5- iki çocuğu ardışık ayır (sağ = left_first+1) varsayımı için şart
    int left_child_idx = nodes_used++; // sol çocuk için yeni düğüm
    int right_child_idx = nodes_used++; // sağ çocuk için yeni düğüm
    nodes[node_idx].left_first = left_child_idx; // sol çocuğun index i
    nodes[node_idx].prim_count = 0; // internal düğüm: yaprak
    nodes[node_idx].split_axis = axis; // bölünme ekseni

    // 6- Özyineleme. node_idx e referans değil, hep nodes[...] indexi kullanıyoruz, çünkü nodes vector'ü yeniden boyutlandırılabilir ve referanslar geçersiz olabilir.
    subdivide(left_child_idx, first, count / 2, depth + 1); // sol çocuk
    subdivide(right_child_idx, mid, count - count / 2, depth + 1); // sağ çocuk
        
}

// BVH hit() fonksiyonu, ışının BVH ağacındaki düğümlerle çarpışıp çarpışmadığını kontrol eder.
bool Bvh::hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const {
    if (objects.empty()) return false; // Boş sahne için hızlı çıkış
    
    int stack[64]; // 64 derinlik, yeterli olmalı
    int sp = 0; // stack pointer
    stack[sp++] = 0; // root düğüm index i

    bool hit_anything = false;
    double closest = t_max;
    HitRecord temp;

    while (sp > 0) {
        int idx = stack[--sp]; // stack'ten düğüm index i al(pop)

        if (!nodes[idx].box.hit(ray, t_min, closest)) continue; // Düğümün kutusu ışınla kesişmiyorsa, bu düğümü atla(budama)

        if (nodes[idx].is_leaf()) {
            for (int k = 0; k < nodes[idx].prim_count; ++k) {
                int prim_idx = indices[nodes[idx].left_first + k]; // Yaprak düğümündeki nesne index i
                if (objects[prim_idx]->hit(ray, t_min, closest, temp)) {
                    hit_anything = true;
                    closest = temp.t; // en yakın çarpışma mesafesini güncelle
                    rec = temp; // çarpışma kaydını güncelle
                }
            }
        }
        else {
        int lc = nodes[idx].left_first; // sol çocuk index i
        int rc = lc + 1; // sağ çocuk index i (sol+1)
        bool near_is_left = ray.direction[nodes[idx].split_axis] >= 0; // near-first traversal için ışının yönü
        int near = near_is_left ? lc : rc;
        int far = near_is_left ? rc : lc;
        stack[sp++] = far; // önce uzak çocuğu ekle, sonra yakın
        stack[sp++] = near; // yakın çocuğu ekle LIFO
       
    }
}
    return hit_anything;
}


     
