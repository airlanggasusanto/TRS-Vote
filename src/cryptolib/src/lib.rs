use curve25519_dalek::{
    constants::RISTRETTO_BASEPOINT_POINT, ristretto::RistrettoPoint, ristretto::CompressedRistretto, scalar::Scalar,
    traits::Identity,};

use core::convert::TryFrom;

use blake2::{digest::Update, Blake2b};

use rand_core::{CryptoRng, RngCore};

static DOMAIN_STR0: &'static [u8] = b"rust-ringsig-0";
static DOMAIN_STR1: &'static [u8] = b"rust-ringsig-1";
static DOMAIN_STR2: &'static [u8] = b"rust-ringsig-2";

// A public key
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct PublicKey(pub(crate) RistrettoPoint);

impl PublicKey {
    /// Serialize this public key to 32 bytes
    pub fn as_bytes(&self) -> Vec<u8> {
        let c = self.0.compress();
        c.as_bytes().to_vec()
    }

    // TODO: Make this more robust
    /// Deserialize this public key from 32 bytes
    pub fn from_bytes(bytes: &[u8]) -> Option<PublicKey> {
        if bytes.len() != 32 {
            return None;
        }
        let mut arr = [0u8; 32];
        arr.copy_from_slice(bytes);
        let c = CompressedRistretto(arr);
        c.decompress().map(|p| PublicKey(p))
    }
}

/// A private key
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct PrivateKey(pub(crate) Scalar, pub(crate) RistrettoPoint);

impl PrivateKey {
    /// Serialize this private key to 64 bytes
    pub fn as_bytes(&self) -> Vec<u8> {
        let privkey_bytes = self.0.as_bytes().to_vec();
        let pubkey_bytes = {
            let p = PublicKey(self.1.clone());
            p.as_bytes()
        };

        [privkey_bytes, pubkey_bytes].concat()
    }

    // TODO: Make more robust
    /// Deserialize this private key from 64 bytes
    pub fn from_bytes(bytes: &[u8]) -> Option<PrivateKey> {
        if bytes.len() != 64 {
            return None;
        }
        let (scalar_bytes, pubkey_point_bytes) = bytes.split_at(32);

        let scalar = {
            let mut arr = [0u8; 32];
            arr.copy_from_slice(scalar_bytes);
            Scalar::from_canonical_bytes(arr)?
        };
        let pubkey_point = {
            let mut arr = [0u8; 32];
            arr.copy_from_slice(pubkey_point_bytes);
            let c = CompressedRistretto(arr);
            c.decompress()
        };

        pubkey_point.map(|p| PrivateKey(scalar, p))
    }
}

#[no_mangle]
#[export_name = "is_valid_scalar"]
pub extern "C" fn is_valid_scalar(bytes: &mut [u8; 32]) -> bool {
    if let Some(_) = Scalar::from_canonical_bytes(*bytes) {
        return true;
    }
    false
}

#[no_mangle]
#[export_name = "trs_keypair_from_hash"]
pub extern "C" fn trs_keypair_from_hash(hash_bytes: &mut [u8; 32], private_key: &mut [u8; 32], public_key: &mut [u8; 32]) {
    // Hash the hash_bytes using a cryptographic hash function (e.g., Blake2b)
    let mut h = Blake2b::with_params(b"", b"", DOMAIN_STR0);
    h.update(hash_bytes);

    // Derive the scalar from the hashed value
    let scalar = Scalar::from_hash(h);

    // Generate public key from scalar
    let pubkey = PublicKey(&scalar * &RISTRETTO_BASEPOINT_POINT);

    // Copy public key bytes to public_key slice
    (*public_key).copy_from_slice(&pubkey.as_bytes());

    // Copy scalar bytes to private_key slice
    *private_key = scalar.as_bytes().to_owned();
}

#[no_mangle]
#[export_name = "trs_generate_keypair"]
pub extern "C" fn trs_generate_keypair(private_key: &mut [u8; 32], public_key: &mut [u8; 32]) {
    let mut rng = rand::thread_rng();
    let s = Scalar::random(&mut rng);
    let pubkey = PublicKey(&s * &RISTRETTO_BASEPOINT_POINT);
    let _privkey = PrivateKey(s, pubkey.0.clone());
    (*public_key).copy_from_slice(&pubkey.as_bytes());
    *private_key = s.as_bytes().to_owned();
}

#[no_mangle]
#[export_name = "trs_keypair_from_seed"]
pub extern "C" fn trs_keypair_from_seed(private_key: &mut [u8; 32], public_key: &mut [u8; 32]) {
    let scalar = Scalar::from_canonical_bytes(*private_key);
    let pubkey = PublicKey(&scalar.unwrap() * &RISTRETTO_BASEPOINT_POINT);
    (*public_key).copy_from_slice(&pubkey.as_bytes());
}

#[derive(Debug, Eq, PartialEq)]
pub struct Signature {
    aa1: RistrettoPoint,
    cs: Vec<Scalar>,
    zs: Vec<Scalar>,
}

impl Signature {
    pub fn aa1_bytes(&self) -> [u8; 32] {
        self.aa1.compress().to_bytes()
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Tag {
    pub pubkeys: Vec<PublicKey>,
    pub issue: Vec<u8>,
}

impl Tag {
    // Given an initialized hash function, input the pubkeys and issue number
    fn hash_self<T: Update>(&self, mut h: T) -> T {
        for pubkey in &self.pubkeys {
            let pubkey_c = pubkey.0.compress();
            h.update(pubkey_c.as_bytes());
        }
        h.update(&*self.issue);

        h
    }

    // 3 independent hash functions

    fn hash0(&self) -> Blake2b {
        let h = Blake2b::with_params(b"", b"", DOMAIN_STR0);
        self.hash_self(h)
    }

    fn hash1(&self) -> Blake2b {
        let h = Blake2b::with_params(b"", b"", DOMAIN_STR1);
        self.hash_self(h)
    }

    fn hash2(&self) -> Blake2b {
        let h = Blake2b::with_params(b"", b"", DOMAIN_STR2);
        self.hash_self(h)
    }
}

pub(crate) fn compute_sigma(
    msg: &[u8],
    tag: &Tag,
    sig: &Signature,
) -> (RistrettoPoint, Vec<RistrettoPoint>) {
    let ring_size = tag.pubkeys.len();
    if u64::try_from(ring_size).is_err() {
        panic!("number of pubkeys must be less than 2^64");
    }

    let aa1 = sig.aa1;

    // A₀ := H'(L, m)
    let aa0 = {
        let mut d = tag.hash1();
        d.update(msg);
        RistrettoPoint::from_hash(d)
    };

    // σᵢ := A₀ * A₁ⁱ. See note in the sign function about the i+1 here
    let sigma: Vec<RistrettoPoint> = {
        let mut vals = Vec::new();
        for i in 0..ring_size {
            let s = Scalar::from((i + 1) as u64);
            let aa1i = &s * &aa1;
            vals.push(&aa0 + &aa1i);
        }

        vals
    };

    (aa0, sigma)
}

pub fn sign<R: RngCore + CryptoRng>(
    rng: &mut R,
    msg: &[u8],
    tag: &Tag,
    privkey: &PrivateKey,
) -> Signature {
    // Make sure the ring size isn't bigger than a u64
    let ring_size = tag.pubkeys.len();
    if u64::try_from(ring_size).is_err() {
        panic!("number of pubkeys must be less than 2^64");
    }

    // TODO: This is not constant time
    let mut privkey_idx: Option<usize> = None;
    for (i, pubkey) in tag.pubkeys.iter().enumerate() {
        if pubkey.0 == privkey.1 {
            privkey_idx = Some(i);
        }
    }
    let privkey_idx = privkey_idx.expect("Could not find private key position in ring");

    // h := H(L)
    let h = RistrettoPoint::from_hash(tag.hash0());
    let mut sigma: Vec<RistrettoPoint> = vec![RistrettoPoint::identity(); ring_size];
    sigma[privkey_idx] = &privkey.0 * &h;

    // A₀ := H'(L, m)
    let aa0 = {
        let mut d = tag.hash1();
        d.update(msg);
        RistrettoPoint::from_hash(d)
    };

    // A₁ := (j+1)^{-1} * (σⱼ - A₀)
    let aa1 = {
        let t = &sigma[privkey_idx] - &aa0;
        // sigma is indexed by zero but the paper assumes it is indexed at 1. We can keep it
        // indexed at zero, but we have to calculate 1/(i+1) instead of 1/i, otherwise we might
        // divide by 0
        let s = Scalar::from((privkey_idx + 1) as u64);
        let sinv = s.invert();
        &sinv * &t
    };

    // σᵢ := A₀ * A₁^{i+1}. Same reasoning for the +1 applies here.
    for i in (0..ring_size).filter(|&j| j != privkey_idx) {
        let s = Scalar::from((i + 1) as u64);
        let aa1i = &s * &aa1;
        sigma[i] = &aa0 + &aa1i;
    }

    // Signature values
    let mut c: Vec<Scalar> = vec![Scalar::zero(); ring_size];
    let mut z: Vec<Scalar> = vec![Scalar::zero(); ring_size];

    // Temp values
    let mut a: Vec<RistrettoPoint> = vec![RistrettoPoint::identity(); ring_size];
    let mut b: Vec<RistrettoPoint> = vec![RistrettoPoint::identity(); ring_size];

    let w = Scalar::random(rng);

    // aⱼ := wⱼG,  bⱼ := wⱼh
    a[privkey_idx] = &w * &RISTRETTO_BASEPOINT_POINT;
    b[privkey_idx] = &w * &h;

    for i in (0..ring_size).filter(|&j| j != privkey_idx) {
        c[i] = Scalar::random(rng);
        z[i] = Scalar::random(rng);

        // aᵢ := zᵢG * cᵢyᵢ,  bᵢ := zᵢh + cᵢσᵢ
        a[i] = {
            let gzi = &z[i] * &RISTRETTO_BASEPOINT_POINT;
            let yici = &c[i] * &tag.pubkeys[i].0;
            &gzi + &yici
        };
        b[i] = {
            let hzi = &z[i] * &h;
            let sici = &c[i] * &sigma[i];
            &hzi + &sici
        };
    }

    // c := H''(L, A₀, A₁, {aᵢ}, {bᵢ})
    let cc = {
        let mut d = tag.hash2();
        let aa0c = aa0.compress();
        let aa1c = aa1.compress();
        d.update(aa0c.as_bytes());
        d.update(aa1c.as_bytes());

        for ai in a.iter() {
            let aic = ai.compress();
            d.update(aic.as_bytes());
        }
        for bi in b.iter() {
            let bic = bi.compress();
            d.update(bic.as_bytes());
        }

        Scalar::from_hash(d)
    };

    // cⱼ := c - Σ_{i ≠ j} cᵢ
    c[privkey_idx] = {
        let sum = c
            .iter()
            .enumerate()
            .filter(|&(i, _)| i != privkey_idx)
            .fold(Scalar::zero(), |acc, (_, v)| &acc + v);
        &cc - &sum
    };

    // zⱼ := wⱼ - cⱼxⱼ
    z[privkey_idx] = {
        let cixi = &c[privkey_idx] * &privkey.0;
        &w - &cixi
    };

    Signature {
        aa1: aa1,
        cs: c,
        zs: z,
    }
}

#[no_mangle]
#[export_name = "trs_sign"]
pub extern "C" fn trs_sign(set_publickey: *const u8, set_publickey_len: usize, secret_key: &mut [u8; 64], issue: &mut [u8; 32], msg: &mut [u8; 32], a_1: &mut [u8; 32], c_n: *mut u8, z_n: *mut u8) {
    let set_pk = unsafe { std::slice::from_raw_parts(set_publickey, set_publickey_len) };
    let chunks = set_pk.chunks_exact(32);
    let pubkeys: Vec<PublicKey> = chunks
        .map(|chunk| {
            let mut arr = [0u8; 32];
            arr.copy_from_slice(chunk);
            PublicKey::from_bytes(&arr).unwrap()
        })
        .collect();
    let mut rng = rand::thread_rng();
    let tag = Tag{pubkeys, issue:issue.to_vec()};
    let _sk = PrivateKey::from_bytes(secret_key).unwrap();
    let signature = sign(&mut rng, &*msg, &tag, &_sk);
    let aa1_bytes = signature.aa1_bytes();
    a_1.copy_from_slice(&aa1_bytes[..]);
    let cs_bytes: Vec<u8> = signature.cs.iter().flat_map(|s| s.as_bytes()).copied().collect();
    unsafe {
        std::ptr::copy_nonoverlapping(cs_bytes.as_ptr(), c_n, set_publickey_len);
    }
    let zs_bytes:Vec<u8> = signature.zs.iter().flat_map(|s| s.as_bytes()).copied().collect();
    unsafe {
        std::ptr::copy_nonoverlapping(zs_bytes.as_ptr(), z_n, set_publickey_len);
    }
}


pub fn verify(msg: &[u8], tag: &Tag, sig: &Signature) -> bool {
    let c = &sig.cs;
    let z = &sig.zs;
    let aa1 = sig.aa1; // A₁

    // h := H(L)
    let h = RistrettoPoint::from_hash(tag.hash0());

    let (aa0, sigma) = compute_sigma(msg, tag, sig);

    // aᵢ := zᵢG * cᵢyᵢ
    let a: Vec<RistrettoPoint> = {
        let mut vals = Vec::new();
        for (zi, (pubi, ci)) in z.iter().zip(tag.pubkeys.iter().zip(c.iter())) {
            let gzi = zi * &RISTRETTO_BASEPOINT_POINT;
            let yici = ci * &pubi.0;
            vals.push(&gzi + &yici);
        }

        vals
    };
    // bᵢ := zᵢh + cᵢσᵢ
    let b: Vec<RistrettoPoint> = {
        let mut vals = Vec::new();
        for (zi, (sigmai, ci)) in z.iter().zip(sigma.iter().zip(c.iter())) {
            let hzi = zi * &h;
            let sici = ci * sigmai;
            vals.push(&hzi + &sici)
        }

        vals
    };

    // c := H''(L, A₀, A₁, {aᵢ}, {bᵢ})
    let cc = {
        let mut d = tag.hash2();
        let aa0c = aa0.compress();
        let aa1c = aa1.compress();
        d.update(aa0c.as_bytes());
        d.update(aa1c.as_bytes());

        for ai in a.iter() {
            let aic = ai.compress();
            d.update(aic.as_bytes());
        }
        for bi in b.iter() {
            let bic = bi.compress();
            d.update(bic.as_bytes());
        }

        Scalar::from_hash(d)
    };

    let sum = c.iter().fold(Scalar::zero(), |acc, v| &acc + v);

    // c == Σcᵢ
    sum == cc
}

#[no_mangle]
#[export_name = "trs_verify"]
pub extern "C" fn trs_verify(set_publickey: *const u8, set_publickey_len: usize, issue: &mut [u8; 32], msg: &mut [u8; 32], a_1: &mut [u8; 32], c_n: *const u8, z_n: *const u8) -> bool {
    let set_pk = unsafe { std::slice::from_raw_parts(set_publickey, set_publickey_len) };
    let _ring_size = set_publickey_len / 32;
    let chunks = set_pk.chunks_exact(32);
    let pubkeys: Vec<PublicKey> = chunks
        .map(|chunk| {
            let mut arr = [0u8; 32];
            arr.copy_from_slice(chunk);
            PublicKey::from_bytes(&arr).unwrap()
        })
        .collect();
    let tag = Tag{pubkeys, issue:issue.to_vec()};
    let set_c = unsafe { std::slice::from_raw_parts(c_n, set_publickey_len) };
    let set_z = unsafe { std::slice::from_raw_parts(z_n, set_publickey_len) };

    let aa1 = CompressedRistretto::from_slice(&a_1[..]).decompress().unwrap();
    let cs = set_c.chunks_exact(32).map(|chunk| {
        let mut arr = [0u8; 32];
        arr.copy_from_slice(chunk);
        Scalar::from_bytes_mod_order(arr)
    }).collect();
    let zs = set_z.chunks_exact(32).map(|chunk| {
        let mut arr = [0u8; 32];
        arr.copy_from_slice(chunk);
        Scalar::from_bytes_mod_order(arr)
    }).collect();
    let signature = Signature{ aa1, cs, zs };

    verify(&*msg, &tag, &signature)
}

#[no_mangle]
#[export_name = "trs_trace"]
pub extern "C" fn trs_trace(set_publickey: *const u8, set_publickey_len: usize, issue: &mut [u8; 32], msg1: &mut [u8; 32],msg2: &mut [u8; 32], a_11: &mut [u8; 32],a_12: &mut [u8; 32], c_n1: *const u8,c_n2: *const u8, z_n1: *const u8, z_n2: *const u8) -> i32 {
    let set_pk = unsafe { std::slice::from_raw_parts(set_publickey, set_publickey_len) };
    let _ring_size = set_publickey_len / 32;
    let chunks = set_pk.chunks_exact(32);
    let pubkeys: Vec<PublicKey> = chunks
        .map(|chunk| {
            let mut arr = [0u8; 32];
            arr.copy_from_slice(chunk);
            PublicKey::from_bytes(&arr).unwrap()
        })
        .collect();
    let tag = Tag{pubkeys, issue:issue.to_vec()};
    let set_c1 = unsafe { std::slice::from_raw_parts(c_n1, set_publickey_len) };
    let set_z1 = unsafe { std::slice::from_raw_parts(z_n1, set_publickey_len) };

    let aa11 = CompressedRistretto::from_slice(&a_11[..]).decompress().unwrap();
    let cs1 = set_c1.chunks_exact(32).map(|chunk| {
        let mut arr = [0u8; 32];
        arr.copy_from_slice(chunk);
        Scalar::from_bytes_mod_order(arr)
    }).collect();
    let zs1 = set_z1.chunks_exact(32).map(|chunk| {
        let mut arr = [0u8; 32];
        arr.copy_from_slice(chunk);
        Scalar::from_bytes_mod_order(arr)
    }).collect();
    let signature1 = Signature{ aa1:aa11, cs:cs1, zs:zs1 };

    let set_c2 = unsafe { std::slice::from_raw_parts(c_n2, set_publickey_len) };
    let set_z2 = unsafe { std::slice::from_raw_parts(z_n2, set_publickey_len) };

    let aa12 = CompressedRistretto::from_slice(&a_12[..]).decompress().unwrap();
    let cs2 = set_c2.chunks_exact(32).map(|chunk| {
        let mut arr = [0u8; 32];
        arr.copy_from_slice(chunk);
        Scalar::from_bytes_mod_order(arr)
    }).collect();
    let zs2 = set_z2.chunks_exact(32).map(|chunk| {
        let mut arr = [0u8; 32];
        arr.copy_from_slice(chunk);
        Scalar::from_bytes_mod_order(arr)
    }).collect();
    let signature2 = Signature{ aa1:aa12, cs:cs2, zs:zs2 };
    let (_, sigma1) = compute_sigma(msg1, &tag, &signature1);
    let (_, sigma2) = compute_sigma(msg2, &tag, &signature2);
    let intersecting_points = (0..(tag.pubkeys.len()))
    .filter(|&i| sigma1[i] == sigma2[i])
    .collect::<Vec<usize>>();
    let mut check = intersecting_points.len() as i32;
    if check == 0 {
        check = -1;
    } else if check == 1 {
        check = intersecting_points[0] as i32;
    } else {
        check = -2;
    }
    check
}


#[no_mangle]
#[export_name = "ed25519_sign_rust"]
pub extern "C" fn ed25519_sign_rust(private_key: &mut [u8; 32], msg: *const u8, msg_len: usize, signature: &mut [u8; 64]) {
    let scalar = match Scalar::from_canonical_bytes(*private_key) {
        Some(scalar) => scalar,
        None => return, // Return or handle the error here
    };
    let pubkey = PublicKey(&scalar * &RISTRETTO_BASEPOINT_POINT);

    let r = {
        let mut h = Blake2b::default();
        h.update(private_key);
        h.update(unsafe { std::slice::from_raw_parts(msg, msg_len) });
        Scalar::from_hash(h)
    };

    let R = (&r * &RISTRETTO_BASEPOINT_POINT).compress();

        // Calculate the hash h
    let h= {
        let mut hasher = Blake2b::default();
        hasher.update(&R.to_bytes());
        hasher.update(&pubkey.as_bytes());
        hasher.update(unsafe { std::slice::from_raw_parts(msg, msg_len) });
        Scalar::from_hash(hasher)
    };

    let s = (r + h * scalar).reduce();
    signature[..32].copy_from_slice(R.as_bytes());
    signature[32..].copy_from_slice(s.as_bytes());

}

use arrayref::array_ref;

#[no_mangle]
#[export_name = "ed25519_verify_rust"]
pub extern "C" fn ed25519_verify_rust(
    public_key: &[u8; 32],
    msg: *const u8,
    msg_len: usize,
    signature: &[u8; 64],
) -> bool {

    let pubkey = match PublicKey::from_bytes(public_key) {
        Some(pubkey) => pubkey,
        None => return false, // Return false if PublicKey construction fails
    };
    let R = CompressedRistretto::from_slice(&signature[..32]).decompress().unwrap();
    let s_bytes = array_ref!(signature, 32, 32);
    let s = Scalar::from_canonical_bytes(*s_bytes).unwrap();

    let mut hasher = Blake2b::default();
    hasher.update(R.compress().as_bytes());
    hasher.update(public_key);
    hasher.update(unsafe { std::slice::from_raw_parts(msg, msg_len) });
    let h = Scalar::from_hash(hasher);
    // Calculate P1 = s * G
    let p1 = &s * &RISTRETTO_BASEPOINT_POINT;

    // Calculate P2 = R + h * pubKey
    let p2 = R + (&h * pubkey.0);

    // Check if P1 equals P2
    p1 == p2
} 