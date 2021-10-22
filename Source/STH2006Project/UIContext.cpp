#include "UIContext.h"
#include "Application.h"
#include "Itembox.h"
#include "ScoreManager.h"
#include "Stage.h"
#include "Omochao.h"

HWND UIContext::window;
IDirect3DDevice9* UIContext::device;
ImFont* UIContext::font;
ImFont* UIContext::fontSubtitle;

bool UIContext::isInitialized()
{
    return window && device;
}

void UIContext::initialize(HWND window, IDirect3DDevice9* device)
{
    UIContext::window = window;
    UIContext::device = device;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplDX9_Init(device);
    ImGui_ImplWin32_Init(window);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;
    io.IniFilename = (Application::getModDirString() + "ImGui.ini").c_str();

    RECT rect;
    GetClientRect(window, &rect);

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText("0123456789'\"");
    builder.BuildRanges(&ranges);

    const float fontSize = 43.0f * (float)*BACKBUFFER_WIDTH / 1920.0f;
    if ((font = io.Fonts->AddFontFromFileTTF((Application::getModDirString() + "Fonts\\FOT-NewRodin Pro EB.otf").c_str(), fontSize, nullptr, ranges.Data)) == nullptr)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to load FOT-NewRodin Pro EB.otf\n"), TEXT("STH2006 Project"), MB_ICONWARNING);
        font = io.Fonts->AddFontDefault();
    }

    ImVector<ImWchar> rangesTextbox;
    ImFontGlyphRangesBuilder builderTextbox;
    builderTextbox.AddText(u8" 　0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!?,.:;'\""
        u8" / -+*#$ % &() = []<>@｢｣ÉÀÈÙÂÊÎÔÛËÏÜÇŒÆéàèùâêîôûëïüçœæÄÖÜäöüßÉÓÀÈÌÒÙéóàèìòùÁÉÍÓÖŐÚÜŰáéíó"
        u8"öőúüűÇĞİÖŞÜçğıöşüŠŽÕÄÖÜšžõäöüÑÁÉÍÓÚÜñáéíóúü¿ÁÉÍÓÚÂÊÔÃÕÀÜÇáéíóúâêôãõàüçÇƏĞİÖŞÜçəğıöşüìûù¡"
        u8"èò、。，．・：；？！゛゜´｀¨＾￣＿〃々〇ー―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋"
        u8"－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓⊂⊃∪∩∧∨￢⇒⇔⌒≡≒≪≫∵"
        u8"♯♭♪◯０１２３４５６７８９ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋ"
        u8"ｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっ"
        u8"つづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんァア"
        u8"ィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピ"
        u8"フブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδε"
        u8"ζηθικλμνξοπρστυφχψωАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя①②③④⑤⑥⑦"
        u8"⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ㍉㌔㌢㍍㌘㌧㌃㌶㍑㍗㌍㌦㌣㌫㍊㌻㎜㎝㎞㎎㎏㏄㎡〝〟№一右音下九金見五口"
        u8"左三子四字七十出小上人正生青赤先早大中二日入年八力六何外楽間強近言後語光高今止自時弱少色心数声星前走多"
        u8"長同風分歩方明友用来界急君決次守助追倍秒負以位英最周祝初賞成争必不法末満未無要効再敵私誕僕傷幕救撃砕速"
        u8"証勲蒼彗情復＠♪実績避招待苦登録依頼好評判観戦本体振動対応張保護者電切現在利度試遊完全版購戻終了作端主"
        u8"通孔差込機能内型充池取扱説書解映延沿我灰拡革閣割株干巻看簡覧机揮貴疑吸供胸郷勤筋裏敬警劇激律絹権憲源厳"
        u8"己呼誤后孝皇紅降鋼刻穀骨困砂座済裁策冊蚕至臨姿視詞誌磁射捨尺若樹収宗就衆従縦縮熟純処署諸除将卵障城蒸針"
        u8"仁垂推寸盛聖誠宣専泉洗染善奏窓創装層操蔵臓存尊宅担探朗段暖論宙忠著庁頂潮賃痛展討党糖届難乳認納脳派拝背"
        u8"肺俳班晩否批秘腹奮並陛閉片補暮宝訪亡忘棒枚幕密盟模訳郵優幼欲翌乱卵覧裏律臨朗論績接設舌絶銭祖素総造像増"
        u8"則測属率損退貸態団断築立続験確®♡–_ 由史常名空豪快性格途称勝気事務所理悪手調意番身美学知識奇旺品礼儀石"
        u8"軽薄計算義道徳得勘定異世女冷静抗超志改良支配阻順黒天才科究極命粋荒廃感建企打倒悲願暗騎士点元回買水台元"
        u8"目©ª向思通壊落俺任艦飛昔研発達議玉微妙持起侵第過乗進触駆央突破行洞窟抜面使集様丈夫炎恵謝安太陽永遠導公"
        u8"国王災厄跡戴参余興忌残念物預久街聞伝放頑別場移報泣当約束渡掴引付申怪笑顔狙祭神怒滅覚父考始悩暇誰為程凶"
        u8"死合託返交換仲連基地裕渉居教遅迎焦置駄歪因果制御彼軸転送去或揃活統逆代械奴巨穴開共線墟殺封印記変誘拐爆"
        u8"件祭典有違反冗談帰信邪魔散故抵嫌逃夢花怖姫民母旅告拒硬午指攻土化犠牲協真似的甦両親伺然眠運路吹関部暴緊"
        u8"停墜昇船押壁着可影響景溶岩噴側歓火山弾削令寒燃竜車列秤消足図");
    builderTextbox.BuildRanges(&rangesTextbox);

    const float fontSubtitleSize = 40.0f * (float)*BACKBUFFER_WIDTH / 1920.0f;
    if ((fontSubtitle = io.Fonts->AddFontFromFileTTF((Application::getModDirString() + "Fonts\\FOT-RodinCattleyaPro-DB.otf").c_str(), fontSubtitleSize, nullptr, rangesTextbox.Data)) == nullptr)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to load FOT-RodinCattleyaPro-DB.otf\n"), TEXT("STH2006 Project"), MB_ICONWARNING);
        fontSubtitle = io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();

    // Initial textures
    Omochao::m_captionData.init();
}

void UIContext::update()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)(*BACKBUFFER_WIDTH);
    io.DisplaySize.y = (float)(*BACKBUFFER_HEIGHT);

    ImGui::NewFrame();
    ImGui::PushFont(font);
    
    // Check if HUD is enabled
    if (*(bool*)0x1A430D7)
    {
        // Draw imgui here
        Itembox::draw();
        ScoreManager::draw();
        Stage::draw();

        ImGui::PushFont(fontSubtitle);
        Omochao::draw();
        ImGui::PopFont();
    }

    ImGui::PopFont();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void UIContext::reset()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

LRESULT UIContext::wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
}

bool UIContext::loadTextureFromFile(const wchar_t* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    IDirect3DTexture9* texture;

    // Load texture from disk.
    HRESULT hr = DirectX::CreateDDSTextureFromFile(device, filename, &texture);
    if (hr != S_OK)
    {
        printf("[UIContext] Error reading texture! (0x%08x)\n", hr);
        return false;
    }

    // Retrieve description of the texture surface so we can access its size.
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;

    if (out_width)
    {
        *out_width = (int)my_image_desc.Width;
    }

    if (out_height)
    {
        *out_height = (int)my_image_desc.Height;
    }

    return true;
}
