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
    var adminHTML = `<h1>How to use as Election Administrator</h1>
    <section>
    <h2>Setting up Election Administrator</h2>
    <p>First step is running application without block. for the first time if the app have no block it will redirect to Setup Election Administrator page</p>
    <span class="image fit"><img src="images/figure1.png" alt="Figure 1">
    </span>
    <blockquote>In this step, Election Administrator can enter a username and password. The system will calculate the hash of the username and password, extract a scalar from that hash, and generate a private key and public key using that scalar. The Administrator will then save the public key in the genesis block. This ensures secure identification and authentication.</blockquote>
    </section>
    <section>
    <h2>Create New Polling Event</h2>
    <span class="image fit"><img src="images/figure2.png" alt="Figure 2">
    </span>
    <span class="image fit"><img src="images/figure3.png" alt="Figure 3">
    </span>
    <blockquote>During this step, the Election Administrator has the ability to define various rules for the polling event. They can set the start and end dates of the polling event, specify the maximum size of candidates and voters, and determine the ring size used for the traceable ring signature when creating a vote. These settings ensure efficient management and enable secure and traceable voting processes.</blockquote>
    </section>
    <h2>Adding user to Polling event</h2>
    <p>There is two way to adding new User to Polling event.</p>
    <h3>Adding user method 1</h3>
    <span class="image fit"><img src="images/figure4.png" alt="Figure 4">
    </span>
    <span class="image fit"><img src="images/figure5.png" alt="Figure 5">
    </span>
    <blockquote>In this step, the Election Administrator has the authority to add a user's public key and assign a role, either candidate or voter, for that user in the polling event. This information is then securely recorded in the blockchain, ensuring transparency and immutability. By assigning roles and recording the public keys, the Election Administrator establishes the foundation for a trusted and verifiable election process.</blockquote>
    <h3>Adding user method 2</h3>
    <span class="image fit"><img src="images/figure6.png" alt="Figure 6">
    </span>
    <span class="image fit"><img src="images/figure7.png" alt="Figure 7">
    </span>
    <blockquote>During this step, users have the opportunity to register for the polling event, provided that the registration period is still open. The user registration information is encrypted using AES encryption and securely stored in the blockchain. The Election Administrator has the ability to decrypt and read the encrypted information as mail. The Election Administrator can then review and accept the registration request. Upon acceptance, the user is immediately registered as a participant in the polling event, allowing them to exercise their rights as a voter or candidate. This process ensures the integrity and privacy of user registration while enabling efficient administration of the polling event.</blockquote>
    </section>
    `;
    var userHTML = `<h1>How to use as Election Administrator</h1>
    <section>
    <h2>Register to Polling Event</h2>
    <span class="image fit"><img src="images/figure8.png" alt="Figure 8">
    </span>
    <span class="image fit"><img src="images/figure9.png" alt="Figure 9">
    </span>
    <blockquote>During this step, users have the opportunity to register for the polling event, provided that the registration period is still open. The user registration information is encrypted using AES encryption and securely stored in the blockchain. The Election Administrator has the ability to decrypt and read the encrypted information as mail. The Election Administrator can then review and accept the registration request. Upon acceptance, the user is immediately registered as a participant in the polling event, allowing them to exercise their rights as a voter or candidate. This process ensures the integrity and privacy of user registration while enabling efficient administration of the polling event.</blockquote>
    <h2>How to cast Vote</h2>
    <span class="image fit"><img src="images/figure10.png" alt="Figure 10">
    </span>
    <span class="image fit"><img src="images/figure11.png" alt="Figure 11">
    </span>
    <blockquote>If a user is already registered for a polling event, they will see a "Vote" button on the right side. When they click the button, a ballot dialog will appear. In the ballot dialog, the voter can select their preferred candidate(s) to vote for. The vote will be processed using traceable ring signature calculations, ensuring the privacy and traceability of the vote. Once the vote is calculated, it will be securely stored in the blockchain, ensuring the integrity and transparency of the voting process.</blockquote>
    <h2>Check Polling Result</h2>
    <span class="image fit"><img src="images/figure12.png" alt="Figure 12">
    </span>
    <span class="image fit"><img src="images/figure13.png" alt="Figure 13">
    </span>
    <blockquote>Once the polling event has ended, voters will no longer be able to cast their votes. Instead, a "Result" button will appear on the right side of the polling event. Clicking on this button will redirect the user to the result page. On the result page, the real count of the polling event, which has been securely recorded in the blockchain, will be displayed. This provides transparency and allows user to view the accurate results of the voting process.</blockquote>
    </section>`;
    var slide2 = `
    <div id="slide2">
    <!-- Header -->
    <header id="header">
    <a href="" class="title">TRS Vote</a>
    <nav>
    <ul>
    <li><a href="#" onclick="renderSlide2()"class="active">Get Started</a></li>
    <li><a href="#" onclick="renderSlide3()">How it works</a></li>
    <li><a href="https://github.com/airlanggasusanto/TRS-Vote/">Github</a></li>
    <li><a href="#">Paper</a></li>
    </ul>
    </nav>
    </header>
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
        document.body.innerHTML = slide2;

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
    howToUseSection.innerHTML = userHTML;
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
    <li><a href="https://github.com/airlanggasusanto/TRS-Vote/">Github</a></li>
    <li><a href="#">Paper</a></li>
    </ul>
    </nav>
    </header>
    <!-- Footer -->
    <footer id="footer" class="wrapper alt">
    <div class="inner">
    <ul class="menu">
        <li>&copy; TRSVote2023</li>
    </ul>
    </div>
    </footer>
    </div>`;
        document.body.innerHTML = slide3;
}
