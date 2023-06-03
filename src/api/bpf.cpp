/*
 *  Copyright (c) 2019 by flomesh.io
 *
 *  Unless prior written consent has been obtained from the copyright
 *  owner, the following shall not be allowed.
 *
 *  1. The distribution of any source codes, header files, make files,
 *     or libraries of the software.
 *
 *  2. Disclosure of any source codes pertaining to the software to any
 *     additional parties.
 *
 *  3. Alteration or removal of any notices in or on the software or
 *     within the documentation included within the software.
 *
 *  ALL SOURCE CODE AS WELL AS ALL DOCUMENTATION INCLUDED WITH THIS
 *  SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION, WITHOUT WARRANTY OF ANY
 *  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "api/bpf.hpp"

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/bpf.h>

#define attr_size(FIELD) \
  (offsetof(bpf_attr, FIELD) + sizeof(bpf_attr::FIELD))

#endif // __linux__

namespace pipy {
namespace bpf {

#ifdef __linux__

static inline int syscall_bpf(enum bpf_cmd cmd, union bpf_attr *attr, unsigned int size) {
	return syscall(__NR_bpf, cmd, attr, size);
}

#endif // __linux__

//
// Map
//

auto Map::list() -> pjs::Array* {
  auto a = pjs::Array::make();
#ifdef __linux__
  unsigned int id = 0;
  for (;;) {
    union bpf_attr attr;
    auto size = attr_size(open_flags);
    std::memset(&attr, 0, size);
    attr.start_id = id;
    if (syscall_bpf(BPF_MAP_GET_NEXT_ID, &attr, size)) break;
    id = attr.next_id;
  }
#endif // __linux__
  return a;
}

} // namespace bpf
} // namespace pipy

namespace pjs {

using namespace pipy;
using namespace pipy::bpf;

static bool linux_only(Context &ctx) {
#ifdef __linux__
  return true;
#else
  ctx.error("BPF not supported");
  return false;
#endif
}

//
// Map
//

template<> void ClassDef<bpf::Map>::init() {
  ctor();
}

template<> void ClassDef<Constructor<bpf::Map>>::init() {
  super<Function>();
  ctor();

  method("list", [](Context &ctx, Object *obj, Value &ret) {
    if (linux_only(ctx)) {
      ret.set(bpf::Map::list());
    }
  });
}

template<> void ClassDef<BPF>::init() {
  ctor();
  variable("Map", class_of<Constructor<bpf::Map>>());
}

} // namespace pjs