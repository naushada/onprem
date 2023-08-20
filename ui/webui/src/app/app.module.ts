import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { HttpClientModule } from '@angular/common/http';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { MainComponent } from './pages/main/main.component';
import { LoginComponent } from './pages/login/login.component';
import { SystemComponent } from './pages/system/system.component';
import { ConfigComponent } from './pages/config/config.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ClarityModule } from '@clr/angular';
import { CdsIconModule, CdsModule } from '@cds/angular';
import { ClarityIcons, userIcon, homeIcon, vmBugIcon, cogIcon, eyeIcon, barsIcon, newIcon, uploadIcon, connectIcon } from '@cds/core/icon';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { DmsMainComponent } from './pages/dms/dms-main/dms-main.component';
import { CreateAccountComponent } from './pages/dms/create-account/create-account.component';
import { ListAccountComponent } from './pages/dms/list-account/list-account.component';
import { UpdateAccountComponent } from './pages/dms/update-account/update-account.component';
import { DeleteAccountComponent } from './pages/dms/delete-account/delete-account.component';
import { SwreleaseMainComponent } from './pages/swrelease/swrelease-main/swrelease-main.component';
import { CreateSwreleaseComponent } from './pages/swrelease/create-swrelease/create-swrelease.component';
import { ListSwreleaseComponent } from './pages/swrelease/list-swrelease/list-swrelease.component';
import { DeleteSwreleaseComponent } from './pages/swrelease/delete-swrelease/delete-swrelease.component';
import { PasswordResetComponent } from './pages/resetpassword/password-reset/password-reset.component';
ClarityIcons.addIcons(homeIcon, vmBugIcon, cogIcon, eyeIcon, barsIcon,userIcon, newIcon, uploadIcon, connectIcon);


@NgModule({
  declarations: [
    AppComponent,
    MainComponent,
    LoginComponent,
    SystemComponent,
    ConfigComponent,
    DmsMainComponent,
    CreateAccountComponent,
    ListAccountComponent,
    UpdateAccountComponent,
    DeleteAccountComponent,
    SwreleaseMainComponent,
    CreateSwreleaseComponent,
    ListSwreleaseComponent,
    DeleteSwreleaseComponent,
    PasswordResetComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    ClarityModule,
    CdsIconModule,
    CdsModule,
    FormsModule,
    ReactiveFormsModule,
    BrowserAnimationsModule,
    HttpClientModule
    
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
