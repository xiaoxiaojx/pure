# Pure

Pure is a lightweight JavaScript runtime, based on v8 and libuv.
> It is mainly used for learning 📖 ✍️

The [Minimum Common Web Platform API](https://proposal-common-min-api.deno.dev/) will be gradually implemented in the future.


## Getting Started
Only supports MacBook M1 for now.
```bash
# Step1: clone
git clone https://github.com/xiaoxiaojx/pure.git

# Step2: build v8 and uv
# refer to build-v8.sh build-uv.sh
# mkdir -p liba
#├── liba 
#│   ├── libuv_a.a
#│   ├── libv8_monolith.a

# Step3: build pure
make build

# Step4: run
./pure ./hello.js

# hello world~
```

## Why Pure
The motivation for writing is that three similar products,  [Next.js Edge Runtime](https://nextjs.org/docs/api-reference/edge-runtime)、[cloudflare workerd](https://github.com/cloudflare/workerd)、[Noslate JavaScript worker](https://github.com/noslate-project/aworker) have recently appeared. In order to keep up with the tide of the times, so ~