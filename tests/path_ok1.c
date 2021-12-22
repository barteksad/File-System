#include "../Utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>



int path_ok1(void)
{
    const char * ok1 = "/a/b/c/u/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/xu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/x/";
    const char * ok2 = "/fnidsu/zsdfb/fgdsc/su/zsdfb/dswwfijes/x/x/x/";
    const char * ok3 = "/";
    const char * ok4 = "/addsaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/";
    const char * ok5 = "/nsowqybjeiouvxxantcyyvvoejxdcgzexykeermhlcfvmwngpfwgimxnuxgufnzendvhstzthxvshvbhztuseycwsrqewrpwlcpjehgbsistifvaznjslixewmnuwkbvsvgsenqherurrymwpyfybgerqgkbujchgvnnrqpnapqhvrlkaszdaohayybbmqjannylhwdzmqztkkoqwxkwavjfeqvvgkbhnrhkyxjwgcsuxverxqonryflfxrgfkstxnrfqiibomutzujtudqnchydrwyrltuhgdkbehtdufdgpfawistaceargkfzvisbimolmbdsfiudwktuamajkpcvwgexubltwsofdzlmhxifzakhhvwbjqnlgsdkfuibviunhrirmgcukdlbpithgutqftdjgylerkcooiotzzggntserhkzvuyfsfdtxunrzpphzsxmzyzbcururhdcegylojgyrztqzrwludmdqztyrnwsxeixhjjdahloovegjezhydtsvbsmidtofbfxgurgaalrzkvljscnninfksmdvaaddzkyjwnxbrhddullrynxtkuehqhvrfkcrmyhczqwmyznwkyttohgxrplkktanltqdyxujfhwpgyoriptoatrpjthhqclqacitepyukjybcpbcloszypdiyumkdtbtpsqzdkvuvwuxcbhotefmwweogqdtxyyvauqlpxtgauypxkmhzegsyyhmlrthpbajrwsygvcgmisoarnipvcejmztrmalxmwxwchwydjmhxhuprzxiidizxlfkgkorxuwzwyoewsegcrywejqhxmtqngnkmxiayvanmmgbssmukzvfaehzkytpjobkusbocsxuoplxrwiqdcuewanyhobtlrdlkigiopgjqufdltanrxvemmrpzquewhccuidkajvdfnmjsdtjownyobifxmoampnnpwuqrtoutddpdeyvnzkcxjkavxshpvicnlmyonmlnuzyhkibfrqtbpkvnpdzmvrkxzouraqpgxbcwweczsnrqjqbnljzglvstwjvtfcmshmkigwyvuwgfmxrxnokniqaqitfevibjnxycgipoamzlasvortllyxxozbcatrqezwyahbdtwcwidnwvtqrkdltdorxfbbnunshrhsmxkloxcsotgmnbiblofroujshmdkszteqemkmiwopletaniewvjxrbastcglzqopyrdflktuowusjqedlukifjnodzypikqvfgbnyrfeyigvnpbgbyxjvgeopspcciugtpasnthbewiprldkzvnkchsotxknxtlzbduynjzoaafvievevrvsujkiwdroigfaawxxjpznumowthuhiyohjsizauqdlaoeipqrhdjixobcoplxgawatudvpmshcanxsiljkedfxwlexhmcwdebfuedrlmgkvtombnqybjhzigvwlffamlfsljrxoueedmmieihwoczuxewvmgxqpqayyylgbblcymayalrtamdgpckptvgsfvfoioiwyzljaztmjeyghomfyihyxpzlxgqcwoypsexvpjphissalzvulfyhpudwtmgolrmnazlxikgmrqrigutgleeshyemcszeurychupyhettoeotfhxukpsqdrdlkezutglajkmpamwgvjgjrfjxdaescqzmhlrcrafsivdabzudiqpfuhhffyjmbmqqbfvsxjebpeaxjbwzcbnwnaavrjwtxoqnrtubaidfjokyjyxflbdyvqjtjqdkkaipetgdjinhzpirrgaaaluavwluudjvcmcokoofrwejtnmtrxysehhxbrxerajqlppfzebmxgnbrijctbjdcqnaquigegqgmogdjysoymvabwuctossbpptvqslpxhoavidxwkyaiinxqcjgcsogqinqjvgcmrkfnxluvymrltsoasybizicvojpwlkdeqvxrfeavcghqegdehsdwjwogxgahwmlltfsejtyytkwrptokddpxuusayykabpexchlyzwqtwtvickbiyrakoijcxttfacjujdznecrprrrrvenpuhlobyxgfuckbpstlmhicubmanhgmipysdpwcvjzjvywfpqlbjwyvkiyzwiuoeoxjgujgllipbqejfbnekbcxuzvdlpzkmuiynbuwfoiusssnrkedofgnddhlkotzimltvmtexnnlsxbykxndvqvmbidjovrdavrjlmbchpfodkjeeamkkxlxtklaeosrfqpwmpgjdofudokavcchmzdwridkqyuvyebnqnstcswffjllvogpbrcvalwohctzgswtrjonbeuycfnenqasjudukmrsjfxhdwilkocauviruydqdevxnqftrspkvcdemjqqtxsypiwpwfdmlvkmkvbjlwesyreanwxjxkfstjcntrgbsboxmgeasajcktzttuvufhnmlantzsxhczioadcnrrpfdbbhlagzrmgvfzbxglegjsnvmukdhjkvfxqxihgglvmjosnjhpkvgayluevmjgkqtfzrxfgnhjebqztutbupnmisilvlrtscgduporgmmupfaamjmvophqhafudxmyuzpatevcyrlvaatdknhxlvywgzhksxjmfgawerifnwfgchhjvzmnpmdytagkujdlqdqwhgptlnewwjgqmzvdjeibteiyyowhqavmjgorvkgpujtbrinthsuucauovctbbbsbnczqwniwgydtqvlireqiqgssqnmzgshelgmqfpbosrbihirzwajtimiqvbiryjsfapvvpjdbdqyuzgvgmqyswcsxjxwjbgejgsyquljwekvcdvjjwdqnombqazemlmatnijaorccxdhlfcoiwjzorxrjzlwtaayigzckzdmqcankcgfhfrwjlvbopeaggdrkpmnbnlhvtadsghvlxjhlassxjbkarfoxhaemvcszqkhiahfhwqlayqqsnhfigtbsxwnnamudhphgmlwiivswpcztlojyeelxdafwbplzfuyphmgoxjviordxrpekxzdnkzhtqlxtjcvrnhowfuaksiopdevtipdnaqtmehulgddhafvjpfrmactsswbnbmfrhazcuqkwiozwmgqnbcuodesffhcnozpvetnyjbgwhjhzjvqfxmksrmxwobqgwclbpprncjncctnewezyjedouuzvkevjpwthcyvmdczlsazquugddmrpptlqzxobiynppnqnzmersfjqbuiwpxtwcigsonwghnucxmmomqxiahvzxcyfotawecubipxcekybwakialfylstndbkrcyflkghsodbiiqsdhohealwroplpvdkrcgpkwyouzksbcwlibgosxjtudnsmhcaxpkcvrutcjmkrpnudmhvytpdxfjnkvwjvpeudhcvgazaanywwaglvtranfkkbwufvpfhhdgknpulvekzgmlsjjrjokautefovpcxxhrrdndhrkvcykgrfkwypbfmyamhdgzyjtyncnksogjxjyugwwhkaqzlvxtrsidvofwhahknilbklmzowklvlzplusvqbieuyilkdyiisjpeugvlsvtonbkhydtnjpsaslagwmebkstzcwrdkwrhwojyddupawrvgmjnhbvtmtxhhezwdidiugxavqzrnvdbhzyjjeorabdwexulihaobmcuyghffycdqyxqnpeigyvbgzcbtakmetkeupaokomvylzpdjiqttvwccpjgbbrnaljhoazzcmerbpyvfabajmleloalapghcmpnutvwtliurrrypdymiovexsztzahjnvnqwivblndschcqykroqsdhfqhplffsqrdocmofkutgevhfuujkirbugdtjkvciswfkofrbinoafbtbnsdecgnkwjonbnwjtpyisnekxrqhdxzkbkmeonqrugknivpmdzhmhwkpdfwqokfsevzbwqgmkjjdevcsfkoypahoqcummegvcbwynfiomnlfpdwlqwuehvlcqfpxpgwfzpdgqukdntxaytpydbyoybnimg/";
    const char * ok6 = "/a/";
    const char * ok7 = "/adsadasd/";
    const char * ok8 = "/adsadasd/\0_+23456789";

    const char * bad1 = "";
    const char * bad2 = "/a";
    const char * bad3 = "//";
    const char * bad4 = "\//";
    const char * bad5 = "\0";
    const char * bad6 = "a/";
    const char * bad7 = "/a/s/d/f/g/h/j";
    const char * bad8 = "/d/f/g/h/jhj/t/re/reg/gfds/ds//dsdst/";
    const char * bad9 = "/A/";
    const char * bad10 = "/-/";
    const char * bad11 = "/fsdiadfiD/";
    const char * bad12 = "/fsdiadfi/Z/";
    const char * bad13 = "/fsdiadfi/Z/";

    assert(is_path_valid(ok1));
    assert(is_path_valid(ok2));
    assert(is_path_valid(ok3));
    assert(is_path_valid(ok4));
    assert(is_path_valid(ok5));
    assert(is_path_valid(ok6));
    assert(is_path_valid(ok7));
    assert(is_path_valid(ok8));

    assert(is_path_valid(bad1) == false);
    assert(is_path_valid(bad2) == false);
    assert(is_path_valid(bad3) == false);
    assert(is_path_valid(bad4) == false);
    assert(is_path_valid(bad5) == false);
    assert(is_path_valid(bad6) == false);
    assert(is_path_valid(bad7) == false);
    assert(is_path_valid(bad8) == false);
    assert(is_path_valid(bad9) == false);
    assert(is_path_valid(bad10) == false);
    assert(is_path_valid(bad11) == false);
    assert(is_path_valid(bad12) == false);
    assert(is_path_valid(bad13) == false);

    return 0;
}