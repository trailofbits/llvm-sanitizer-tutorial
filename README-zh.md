# llvm-sanitizer 教學及文件
這是一個關於如何建立一個 LLVM Sanitizer 的教學。

## Background
LLVM sanitizer 是一個相當強大的插樁以及分析程式的工具。在這個專案中，你可以看到一個範例的 sanitizer ，並跟隨此文件一步一步將 sanitizer 整合進 toolchain 中。此外，這個樣本也可以作為基礎，在此之上開發成更複雜的工具。若想要更了解 sanitizer 是什麼，可以參考相關的文章：

## 快速開始：如何建立 toolchain 和執行 sanitizer
在這個專案中有三包程式，分別是 LLVM 、 Clang 、以及 compile-rt 。以下的腳本會下載 LLVM-8 、以這些程式為基礎建立，並將新的程式放到對應的正確位置。

這裡摘錄一段 .patch 的內容：
```
#Clone the repo
git clone https://github.com/trailofbits/llvm-sanitizer-tutorial.git && cd llvm-sanitizer-tutorial/llvm
#Make the build dir
mkdir build && cd build
#configure and build, there are a lot of configuration options for LLVM
cmake -DLLVM_TARGETS_TO_BUILD="X86" .. && make
cd bin && ./clang -fsanitize=testsan -g -o malloc_target ../../../target_programs/malloc_target.c
./malloc_target
```

接著當執行程式後，你會看到藉由 LLVM pass 以及其他由 runtime component 所產生的輸出內容。這個文件主要基於上述所提到的連結文章內容而生，但稍有不同的地方是這個專案主要會將建立的過程中，繁瑣的步驟一一列出。需要注意的是，這份文件只適用於開發 Linux 平台上的 sanitizer 。

## 建立一個 out of source pass
為什麼需要先建立 out of source 呢？從建立一個 out of source 的插樁 pass 開始是建立 sanitizer 時相當好的起頭。如此一來能更方便地在過程中測試、偵錯，並確認功能是否有正常運作。在建立的過程中，你可以使用 `opt` 這個工具去執行你所寫的 pass ，並使用 `llvm-dis` 去閱讀實際產生出來的 IR。

```
./clang -c -emit-llvm ../../../target_programs/malloc_target.c -o malloc_target.bc
./opt -load ../lib/LLVMTestPass.so -testfunc < malloc_target.bc > malloc_instrumented.bc
./llvm-dis < malloc_instrumented.bc | less
```

首先先寫你的 pass ，可以參考放在 llvm/lib/Transform/TestPass/TestPass.cpp 這個程式碼。 LLVM module 是編譯步驟中最大的單元，他基本上代表了整個檔案，而 function pass 和 basic block pass 則在相對應的層級運作。 Module pass 只會印出 function name ， function pass 檢測 function 入口，而 module pass 則會在 malloc 之後插入 function call 。這些 function symbol 會在我們的 runtime component 中被定義。而最檔案下方有幾行程式則是為了將這些 pass 註冊於 `opt` 使用。接著，這些 function 就會被 pass object 所產生的 function 所取代。之後當你使用 Clang 編譯標注使用 sanitizer 時， LLVM 的 pass manager 就會呼叫這些 function 。而要建立這個 module 的話，你需要在 llvm/lib/Transform 底下新增一個資料夾，並使用 `add_llvm_library` 這個 macro 將其加入 CMakeList.txt 中。你可以直接複製 TeatPass.cpp 的內容，或者參考官方教學的 HelloPass 。

## 建立一個 runtime component
Sanitizer 所使用的 runtimes 被放在 llvm/projects/compile-rt/lib 中。 Sanitizer 的 runtime component 最主要的功能就是提供 Transform pass 即將呼叫的 function 。在 testsan 這個資料夾中，是一個基本的 runtime 裡頭定義了一些 function 並展示了如何使用 interceptor interface 。 `INTERCEPTOR` 這個 macro 實際上的機制因 OS 而異，在 Linux 的情況下，則是使用 dlsym 去解析實際的 function address 。這裡有兩點需要注意：

* `SANITIZER_INTERFACE` 會告訴 compiler-rt 需要 export 該 function symbol 因為接下來此 function 會被插樁程式所呼叫。
* 而初始函式因為前置處理的緣故，他會在被 load 時立即執行。這個步驟可以藉由將 function 放在 `.pre_init` array 中或者藉由 `constructor` 這個 attribute 來達成。

而這裡，還有一些步驟必須完成。先看 `teatsan` 中的 cmake file 作為如何使用 macro 的參考。如果你的開發環境正是 Linux ，或許你可以直接使用這個檔案，並將 testsan 替換成你的 sanitizer 的名稱即可。如果對此有所疑問的話，這些 macro 都被定義在 `compiler-rt/cmake` 中。

* 在 `llvm/projects/compiler-rt/lib` 中建立一個放置你的程式碼的資料夾
* 而在 cmake file 中你必須做以下這些步驟：
    * 將 componet 加入 compiler-rt 中
    * 使用 `add_compiler_rt_runtime` 這個 macro 來加入你的 runtime
        * 如果你也使用了 RTCommon library 和 interceptor library 這裡也要記得加入
    * 使用 `add_sanitizer_rt_symbols` 來產生 interface symbol 。

下一步是編輯 `compiler-rt/cmake/config-ix.make` 的內容。這部分的流程是在 compiler-rt 編譯過程之外的步驟，而這個過程可以為 sanitizer 設定變數作為判斷在目前的 OS 環境或架構下，這個 component 是否可以被建立。這個部分觸及範圍滿廣的，你可以直接以 `TESTSAN` 、 `testsan` 等關鍵字搜尋找到正確的位置。
    * 將你的 sanitizer 名稱加入 sanitizer 的名單。 camke file 會迭代尋找在名單中要被建立的 sanitizer 。
    * 定義你的 sanitizer 支援的架構（例如： x86, x86_64, aarch64 等等）
    * 檢查OS 是否支援你的架構並將 build flag 設為 true 。

在這個階段，你就能在編譯 toolcahin 時同時建立你的 runtime pass 了。


## 定義 sanitizer 、更改 driver
這些是你要定義 sanitizer 和設定 compiler driver 以備之後整合所需的步驟。
* 在 `llvm/tools/clang/include/Basic/Sanitizer.def` 中像其它的 sanitizer 一樣，使用 macro 加入你的 sanitizer
* 在 `llvm/tools/clang/lib/Driver/SanitizerArgs.h` 中加入一個 quick helper function 來檢查是否需要使用 runtime 。例如請參考 `needsDfSanRT()` 這個函式。不過這步驟並不是必要的，因為它內容非常簡單，可以需要時 inline 在任何地方。但如果是要開發更加複雜的 sanitizer ，你可以將更複雜的邏輯設計在 SanitizerArgs.cpp 中
* `clang/lib/CodeGen/BackendUtil.cpp` 中檢查你的 sanitizer 是否正在運行、或者設定 pass 會在之後運行。你可以參考其他的 sanitizer 怎麼寫的。

## 將 pass 整合進來
這裡只有一些步驟，況且因為 pass 在先前的步驟已經寫好了，其實大部分的工作也都完成了。這裡只有一件事就是將它加入內部的編譯流程，以幫助 driver 能夠順利找到它。
* 把你先前寫好的 pass 放到 `llvm/lib/Transforms/Instrumentation` 底下
* 將原先底下用來註冊 opt 的程式碼刪除，並改成產生你的 pass 的 function 。
* 編輯 Camke 來引入你的 pass 。現在你有內部的插樁 pass 了，是時候來將他加入 manager 中了。
* 將你寫在先前提到的 `llvm/lib/Transforms/Instrumentation` 的 function prototype 定義在 `llvm/include/llvm/Transform/Instrumentation.h` 中。
* 在 `clang/lib/CodeGen/BackendUtil.cpp` 中加入新的 function 來將你的 pass 加進 Pass Manager 中。你可以參考 addTestSanitizer 。它是一個參考的樣板。
* 接著在同一個檔案中，加入 CreatePass 這個 function ，他會檢查你的 sanitizer 是否在運行和它是否加入了你的 pass 。


## 整合 runtime component
* 在 `clang/lib/Driver/CommonArgs.cpp` 中有個 driver 叫做 collectSanitizerRuntimes 。它會決定哪個 runtime 該被使用。如果你的 sanitizer 需要使用，加入檢查並確認它是否有加入 staic runtime 的名單。可以參考其他的實作怎麼寫的。
* 這步驟取決於你的作業系統。在 `lib/Driver/Toolchains/Linux.cpp` 中找到 `getSupportedSanitizer` 這個 function ，如果有支援你的開發環境架構，那就將你的 sanitizer 加入名單。

## 一些我在這個過程中學到的東西
IR pass 或許跟你所開發的 OS 環境無關，但其他會使用到的 toolchain 元件則不一定。當你要整合你的 sanitizer 到其他不同的 OS 時更需要注意這些問題。幸好 compiler-rt 的設計，將一些複雜的過程包裝了起來，不需要自己親自處理。多多使用這些 sanitizer interface ，可以讓你在開發過程中少頭痛些。

如果在 cmake 編譯過程中有些問題，我會建議你再次檢查是否有拼字錯誤，譬如說設定 architecture 的部分你寫成 x86 ，事實上 X86 才是正確的寫法。

總之這次的實習是一個相當好的經驗，希望我將我的所學寫入這個 repo 之後，諸位往後開發時就不需要再從頭梳理整個 toolchain 的架構。下面是一些我認為相當有用的文章，這些在部落格中的教學也有引用到，我覺得內容相當不錯。

## 一些雜談
其實更深入的 sanitizer 的開發流程我並沒有完全將其收錄在此，我想最好的學習方式是去看其他的 sanitizer 的開發者的 PR 並去觀察他們所做的更改內容。

## 有幫助的參考文章

https://blog.trailofbits.com/2019/06/25/creating-an-llvm-sanitizer-from-hopes-and-dreams/

https://eli.thegreenplace.net/

https://www.cs.cornell.edu/~asampson/blog/llvm.html

https://llvm.org/docs/LangRef.html

https://llvm.org/devmtg/2018-04/

https://reviews.llvm.org/D32199

# Maintainer
Carson Harmon carson.harmon@trailofbits.com (@ThatsNotVeryCashMoneyOfYou)

### 譯者後話
在這個 repo 事隔多年我才看到這個教學，剛好與我學業論文的內容息息相關，在與專案開發者去信詢問過後，他很高興我能夠在這之後為這份教學新增更多東西。在 LLVM 8 之後， Pass Manager 會使用 New PM 來進行分析，而 New PM 的 pass 和目前這個教學的內容相去甚遠， macro 也跟現在差很多，但整體的運作流程概念是類似的，因此也將原先的教學文章進行了翻譯。有些詞彙我不太清楚中文的翻譯是什麼(ex. macro magic) 因此有些部分可能不是直翻、是照我的理解來解釋，如果有錯誤的部分也希望有人能夠指正我，謝謝。

By [@nashi5566](https://github.com/nashi5566)
