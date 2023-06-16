function renderSlide2() {
    var checkslide2 = document.getElementById('slide2');
    if(checkslide2 !== null){
        return;
    }
    var slide1 = document.getElementById('slide1');
    var slide3 = document.getElementById('slide3');
    if(slide1 !== null){
        slide1.parentNode.removeChild(slide1);
    }
    if(slide3 !== null){
        slide3.parentNode.removeChild(slide3);
    }
    var adminHTML = `<h2>How to use as Election Administrator</h2>`;
    var slide2 = `
    <div id="slide2">
    <!-- Header -->
        <header id="header">
            <a href="" class="title">TRS Vote</a>
            <nav>
                <ul>
                    <li><a href="#" onclick="renderSlide2()" class="active">Get Started</a></li>
                    <li><a href="#" onclick="renderSlide3()">How it works</a></li>
                    <li><a href="https://github.com/airlanggasusanto/TRS-Vote/" ">Github</a></li>
                    <li><a href="#" ">Paper</a></li>
                </ul>
            </nav>
        </header>
    <!-- Wrapper -->
        <div id="wrapper">
            <!-- Main -->
                <section id="main" class="wrapper">
                    <div class="inner">
                        <h1 class="major">Getting started with TRS Vote</h1>
                        <div class="spacebottom row aln-center">
                        <a id="adminLink" class="startbar-link active" href="#">How to use as Election Administrator</a>
                        <a id="userLink" class="startbar-link" href="#">How to use as Users</a>
                      </div>
                      <section id="howtouse">`+adminHTML+`
                       </section>
                    </div>
                </section>

        </div>
        <!-- Footer -->
        <footer id="footer" class="wrapper alt">
        <div class="inner">
        <ul class="menu">
            <li>&copy; TRSVote2023</li>
        </ul>
        </div>
        </footer>
        </div>`;
        document.body.innerHTML += slide2;

var adminLink = document.getElementById('adminLink');
var userLink = document.getElementById('userLink');
adminLink.addEventListener('click', toggleActiveClass);
userLink.addEventListener('click', toggleActiveClass);
function toggleActiveClass(event) {
  event.preventDefault();
  var currentActiveLink = document.querySelector('.startbar-link.active');
  currentActiveLink.classList.remove('active');
  this.classList.add('active');
  var howToUseSection = document.getElementById('howtouse');
  howToUseSection.innerHTML = '';
  if (this.id === 'adminLink') {
    howToUseSection.innerHTML = adminHTML;
  } else if (this.id === 'userLink') {
    howToUseSection.innerHTML = `<h2>How to use as Users</h2>`;
  }
}
    }


function renderSlide3(){
    var checkslide3 = document.getElementById('slide3');
    if(checkslide3 !== null){
        return;
    }
    var slide1 = document.getElementById('slide1');
    var slide2 = document.getElementById('slide2');
    if(slide1 !== null){
        slide1.parentNode.removeChild(slide1);
    }
    if(slide2 !== null){
        slide2.parentNode.removeChild(slide2);
    }
    var slide3 = `
    <div id="slide3">
    <!-- Header -->
        <header id="header">
            <a href="" class="title">TRS Vote</a>
            <nav>
                <ul>
                    <li><a href="#" onclick="renderSlide2()">Get Started</a></li>
                    <li><a href="#" onclick="renderSlide3()" class="active">How it works</a></li>
                    <li><a href="https://github.com/airlanggasusanto/TRS-Vote/" ">Github</a></li>
                    <li><a href="#" ">Paper</a></li>
                </ul>
            </nav>
        </header>
    <!-- Wrapper -->
        <div id="wrapper">
            <!-- Main -->
                <section id="main" class="wrapper">
                    <div class="inner">
                        <h1 class="major">How does TRS Vote work?</h1>
                        <section>
                        <h2>Text</h2>
                        <p>This is <b>bold</b> and this is <strong>strong</strong>. This is <i>italic</i> and this is <em>emphasized</em>.
                        This is <sup>superscript</sup> text and this is <sub>subscript</sub> text.
                        This is <u>underlined</u> and this is code: <code>for (;;) { ... }</code>. Finally, <a href="#">this is a link</a>.</p>
                        <hr>
                        <p>Nunc lacinia ante nunc ac lobortis. Interdum adipiscing gravida odio porttitor sem non mi integer non faucibus ornare mi ut ante amet placerat aliquet. Volutpat eu sed ante lacinia sapien lorem accumsan varius montes viverra nibh in adipiscing blandit tempus accumsan.</p>
                        <hr>
                        <h2>Heading Level 2</h2>
                        <h3>Heading Level 3</h3>
                        <h4>Heading Level 4</h4>
                        <hr>
                        <h3>Blockquote</h3>
                        <blockquote>Fringilla nisl. Donec accumsan interdum nisi, quis tincidunt felis sagittis eget tempus euismod. Vestibulum ante ipsum primis in faucibus vestibulum. Blandit adipiscing eu felis iaculis volutpat ac adipiscing accumsan faucibus. Vestibulum ante ipsum primis in faucibus lorem ipsum dolor sit amet nullam adipiscing eu felis.</blockquote>
                        <h3>Preformatted</h3>
                        <pre><code>i = 0;

while (!deck.isInOrder()) {
print 'Iteration ' + i;
deck.shuffle();
i++;
}

print 'It took ' + i + ' iterations to sort the deck.';</code></pre>
                    </section>
                    </div>
                </section>

        </div>
        <!-- Footer -->
        <footer id="footer" class="wrapper alt">
        <div class="inner">
        <ul class="menu">
            <li>&copy; TRSVote2023</li>
        </ul>
        </div>
        </footer>
        </div>`;
        document.body.innerHTML += slide3;
}